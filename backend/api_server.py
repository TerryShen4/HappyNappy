"""HappyNappy backend: receives PPG windows over a Bluetooth serial link,
computes BPM, runs sleep-stage detection, and exposes the results over HTTP for
the dashboard.

The heavy lifting lives in sibling modules:
  - config.py        -- tunable constants (serial port, thresholds)
  - processing.py    -- raw IR window -> BPM
  - sleep_tracker.py -- BPM history + wake detection
This file is just the I/O layer that wires them together.

Transport: the ESP32 pairs over Bluetooth Classic (SPP) and shows up as a
virtual COM port on the laptop. The pillow streams one JSON line per window
(`{"ir":[...500...],"sample_rate":50}\n`); we stream `WAKE_UP` / `STOP_SAMPLING`
lines back to it.
"""

import json
import threading
from contextlib import asynccontextmanager
from datetime import datetime

import serial
from fastapi import FastAPI

import config  # importing this also enables UTF-8 console output (see config.py)
from processing import average_ir, calculate_bpm, has_good_contact
from sleep_tracker import SleepTracker

# --- Runtime state ---
tracker = SleepTracker()
state_lock = threading.Lock()       # guards tracker + link_status (reader vs HTTP)

link_status = {
    "connected": False,             # serial port currently open?
    "receiving_packages": False,
    "total_packages": 0,
    "last_updated": None,
}

_serial = None                      # the open pyserial connection (or None)
_serial_lock = threading.Lock()     # guards writes to the port
_stop = threading.Event()           # set on shutdown to stop the reader thread


def send_wake_alert():
    """Tell the ESP32 to wake the sleeper and stop streaming samples."""
    with _serial_lock:
        if _serial is None or not _serial.is_open:
            print("Cannot send wake alert: serial link not open")
            return
        _serial.write((config.MSG_WAKE_UP + "\n").encode())
        _serial.write((config.MSG_STOP_SAMPLING + "\n").encode())
    print("Sent WAKE_UP and STOP_SAMPLING to ESP32")


def process_window(line: str):
    """Handle one PPG window (a single JSON line from the pillow)."""
    try:
        data = json.loads(line)
    except json.JSONDecodeError:
        return  # partial/garbage line -- ignore

    ir_data = data.get("ir")
    sample_rate = data.get("sample_rate")

    with state_lock:
        link_status["receiving_packages"] = True
        link_status["total_packages"] += 1
        link_status["last_updated"] = datetime.now().isoformat()
        count = link_status["total_packages"]

    if not (isinstance(ir_data, list) and ir_data):
        print(f"Package #{count} received | IR: {ir_data}")
        return

    print(
        f"Package #{count} @ {datetime.now():%H:%M:%S} "
        f"-- {len(ir_data)} samples @ {sample_rate}Hz, IR {min(ir_data)}-{max(ir_data)}"
    )

    # Only analyze windows where the sensor is actually on skin.
    if not has_good_contact(ir_data):
        print(f"   Poor contact (avg IR: {average_ir(ir_data):.0f}) - skipping analysis")
        return

    # HeartPy can take a few ms -- compute outside the lock so HTTP stays snappy.
    try:
        bpm = calculate_bpm(ir_data, sample_rate)
    except Exception as hp_error:
        print(f"   HeartPy failed: {hp_error}")
        return

    if bpm is None:
        print("   Invalid BPM value - skipping")
        return

    print(f"   Calculated BPM: {bpm:.2f}")

    # Record the reading; the tracker logs stage changes and tells us when to wake.
    with state_lock:
        events = tracker.record_bpm(bpm)
    if "wake" in events:
        send_wake_alert()


def serial_reader_loop():
    """Background thread: keep the COM port open and feed each line to the pipeline."""
    global _serial
    while not _stop.is_set():
        try:
            with _serial_lock:
                _serial = serial.Serial(config.SERIAL_PORT, config.SERIAL_BAUD, timeout=1)
            with state_lock:
                link_status["connected"] = True
            print("=" * 60)
            print(f"Bluetooth serial link open on {config.SERIAL_PORT}")
            print("=" * 60 + "\n")
        except serial.SerialException as e:
            with state_lock:
                link_status["connected"] = False
            print(
                f"Waiting for {config.SERIAL_PORT} (pair 'HappyNappy' and set "
                f"HAPPYNAPPY_PORT if needed): {e}"
            )
            _stop.wait(config.SERIAL_RECONNECT_SEC)
            continue

        try:
            while not _stop.is_set():
                raw = _serial.readline()  # blocks up to timeout; b"" on timeout
                if not raw:
                    continue
                line = raw.decode(errors="replace").strip()
                if line:
                    process_window(line)
        except serial.SerialException as e:
            print(f"Serial link lost ({e}); will try to reopen...")
        finally:
            with _serial_lock:
                try:
                    if _serial is not None:
                        _serial.close()
                except Exception:
                    pass
                _serial = None
            with state_lock:
                link_status["connected"] = False


@asynccontextmanager
async def lifespan(app: FastAPI):
    """Start the serial reader thread for the lifetime of the server."""
    reader = threading.Thread(target=serial_reader_loop, daemon=True)
    reader.start()
    yield
    _stop.set()


app = FastAPI(title="HappyNappy Sleep Tracker", lifespan=lifespan)


# --- HTTP endpoints (consumed by the Streamlit dashboard) ---

@app.get("/")
async def status():
    """Link/pipeline status summary."""
    with state_lock:
        receiving = link_status["receiving_packages"] or link_status["total_packages"] > 0
        return {
            "link": "Connected" if link_status["connected"] else "Waiting for pillow...",
            "receiving_packages": link_status["receiving_packages"],
            "total_packages": link_status["total_packages"],
            "last_updated": link_status["last_updated"],
            "sleep_stage": tracker.sleep_stage,
            "baseline_bpm": tracker.baseline_bpm,
            "awake_alert_triggered": tracker.awake,
        }


@app.get("/bpm_history")
async def get_bpm_history():
    """BPM history for visualization."""
    with state_lock:
        return {
            "data": list(tracker.bpm_history),
            "total_readings": tracker.total_readings,
            "current_bpm": tracker.current_bpm,
        }


@app.post("/wake_alert")
async def trigger_wake_alert():
    """MANUAL TEST ONLY -- automatic detection happens in process_window."""
    with state_lock:
        tracker.arm_wake()
    try:
        send_wake_alert()
        return {"status": "success", "message": "Wake alert sent to ESP32"}
    except Exception as e:
        print(f"Failed to send wake alert: {e}")
        return {"status": "error", "message": str(e)}


@app.get("/awake_status")
async def get_awake_status():
    """Current awake/alert flag."""
    with state_lock:
        return {"awake": tracker.awake}


if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)
