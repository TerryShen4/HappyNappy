"""HappyNappy backend: receives PPG windows over MQTT, computes BPM, runs
sleep-stage detection, and exposes the results over HTTP for the dashboard.

The heavy lifting lives in sibling modules:
  - config.py        -- tunable constants (broker, topics, thresholds)
  - processing.py    -- raw IR window -> BPM
  - sleep_tracker.py -- BPM history + wake detection
This file is just the I/O layer that wires them together.
"""

import json
from datetime import datetime

from fastapi import FastAPI
from fastapi_mqtt import FastMQTT, MQTTConfig

import config  # importing this also enables UTF-8 console output (see config.py)
from processing import average_ir, calculate_bpm, has_good_contact
from sleep_tracker import SleepTracker

# --- App + MQTT setup ---
app = FastAPI(title="HappyNappy Sleep Tracker")

mqtt = FastMQTT(config=MQTTConfig(
    host=config.MQTT_HOST,
    port=config.MQTT_PORT,
    keepalive=config.MQTT_KEEPALIVE,
))
mqtt.init_app(app)

# --- Runtime state ---
tracker = SleepTracker()
mqtt_status = {
    "receiving_packages": False,
    "total_packages": 0,
    "last_updated": None,
}


def send_wake_alert():
    """Tell the ESP32 to wake the sleeper and stop streaming samples."""
    mqtt.client.publish(config.TOPIC_WAKE_ALERT, config.MSG_WAKE_UP, qos=0)
    mqtt.client.publish(config.TOPIC_CONTROL, config.MSG_STOP_SAMPLING, qos=0)
    print("📤 Sent WAKE_UP and STOP_SAMPLING to ESP32")


# --- MQTT handlers ---

@mqtt.on_connect()
def on_connect(client, flags, rc, properties):
    print("=" * 60)
    print("✅ Connected to MQTT Broker!")
    print(f"📡 Subscribed to: {config.TOPIC_SENSOR_DATA}")
    print("=" * 60 + "\n")
    mqtt.client.subscribe(config.TOPIC_SENSOR_DATA)


@mqtt.on_message()
async def on_message(client, topic, payload, qos, properties):
    """Handle one PPG window from the ESP32."""
    try:
        data = json.loads(payload.decode())

        mqtt_status["receiving_packages"] = True
        mqtt_status["total_packages"] += 1
        mqtt_status["last_updated"] = datetime.now().isoformat()

        ir_data = data.get("ir")
        sample_rate = data.get("sample_rate")

        if not (isinstance(ir_data, list) and ir_data):
            print(f"📩 Package #{mqtt_status['total_packages']} received | IR: {ir_data}")
            return

        print(
            f"📩 Package #{mqtt_status['total_packages']} @ {datetime.now():%H:%M:%S} "
            f"-- {len(ir_data)} samples @ {sample_rate}Hz, IR {min(ir_data)}-{max(ir_data)}"
        )

        # Only analyze windows where the sensor is actually on skin.
        if not has_good_contact(ir_data):
            print(f"   ⚠️ Poor contact (avg IR: {average_ir(ir_data):.0f}) - skipping analysis")
            return

        try:
            bpm = calculate_bpm(ir_data, sample_rate)
        except Exception as hp_error:
            print(f"   ⚠️ HeartPy failed: {hp_error}")
            return

        if bpm is None:
            print("   ⚠️ Invalid BPM value - skipping")
            return

        print(f"   💓 Calculated BPM: {bpm:.2f}")

        # Record the reading; the tracker logs stage changes and tells us when to wake.
        if "wake" in tracker.record_bpm(bpm):
            send_wake_alert()

    except Exception as e:
        print(f"❌ Error: {e}")


# --- HTTP endpoints (consumed by the Streamlit dashboard) ---

@app.get("/")
async def status():
    """MQTT/pipeline status summary."""
    receiving = mqtt_status["receiving_packages"] or mqtt_status["total_packages"] > 0
    return {
        "mqtt_broker": "Connected" if receiving else "Waiting for data...",
        "receiving_packages": mqtt_status["receiving_packages"],
        "total_packages": mqtt_status["total_packages"],
        "last_updated": mqtt_status["last_updated"],
        "sleep_stage": tracker.sleep_stage,
        "baseline_bpm": tracker.baseline_bpm,
        "awake_alert_triggered": tracker.awake,
    }


@app.get("/bpm_history")
async def get_bpm_history():
    """BPM history for visualization."""
    return {
        "data": tracker.bpm_history,
        "total_readings": tracker.total_readings,
        "current_bpm": tracker.current_bpm,
    }


@app.post("/wake_alert")
async def trigger_wake_alert():
    """MANUAL TEST ONLY -- automatic detection happens in on_message."""
    tracker.arm_wake()
    try:
        send_wake_alert()
        return {"status": "success", "message": "Wake alert sent to ESP32"}
    except Exception as e:
        print(f"❌ Failed to send wake alert: {e}")
        return {"status": "error", "message": str(e)}


@app.get("/awake_status")
async def get_awake_status():
    """Current awake/alert flag."""
    return {"awake": tracker.awake}


if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)
