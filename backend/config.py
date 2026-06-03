"""Central configuration for the HappyNappy backend.

Tunable constants live here so they're easy to find and adjust without
digging through the request/processing code.
"""

import os
import sys

# Windows consoles default to cp1252, which raises UnicodeEncodeError when a
# print() contains emoji (our log messages do). Every backend module imports
# this one, so enabling UTF-8 here makes logging safe regardless of which
# script is the entry point.
try:
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    sys.stderr.reconfigure(encoding="utf-8", errors="replace")
except Exception:
    pass

# --- Bluetooth serial link (Classic SPP) ---
# Pair the pillow ("HappyNappy") in Windows Bluetooth settings; Windows creates
# an OUTGOING virtual COM port. Point the backend at it here, or override at
# launch:  set HAPPYNAPPY_PORT=COM7   (Windows)  /  export HAPPYNAPPY_PORT=...
SERIAL_PORT = os.environ.get("HAPPYNAPPY_PORT", "COM5")
SERIAL_BAUD = 115200          # ignored by SPP, but pyserial requires a value
SERIAL_RECONNECT_SEC = 3      # wait between reopen attempts if the port is gone

# --- Commands sent back to the ESP32 (newline-framed lines over the link) ---
MSG_WAKE_UP = "WAKE_UP"
MSG_STOP_SAMPLING = "STOP_SAMPLING"

# --- Signal processing ---
MIN_CONTACT_IR = 900            # avg IR below this => finger not on the sensor
# Bandpass to clean the PPG. The UPPER cutoff doubles as a sanity ceiling: set
# too high (e.g. 3.5 Hz / 210 BPM) it lets the dicrotic-notch + noise above the
# pulse get counted as extra beats, which roughly doubled the reported BPM. For a
# resting nap pillow, 2.0 Hz (~120 BPM) keeps real heart rates and rejects that
# high-frequency junk, so HeartPy locks onto the true beat.
BANDPASS_CUTOFF = [0.7, 2.0]    # Hz bandpass (~42-120 BPM)
BANDPASS_ORDER = 3

# --- Sleep-stage detection ---
BASELINE_READINGS = 60          # readings averaged to set the resting baseline
DEEP_SLEEP_DROP_PCT = 20        # % BPM drop from baseline => deep sleep (wake!)
FALLING_ASLEEP_DROP_PCT = 10    # % BPM drop from baseline => falling asleep
