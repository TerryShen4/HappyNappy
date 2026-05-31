"""Central configuration for the HappyNappy backend.

Tunable constants live here so they're easy to find and adjust without
digging through the request/processing code.
"""

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

# --- MQTT broker ---
MQTT_HOST = "localhost"
MQTT_PORT = 1883
MQTT_KEEPALIVE = 60

# --- MQTT topics ---
TOPIC_SENSOR_DATA = "sensors/max30102/data"  # ESP32 -> backend (PPG windows)
TOPIC_WAKE_ALERT = "esp32/wake_alert"        # backend -> ESP32 (buzz/wake)
TOPIC_CONTROL = "esp32/control"              # backend -> ESP32 (commands)
MSG_WAKE_UP = "WAKE_UP"
MSG_STOP_SAMPLING = "STOP_SAMPLING"

# --- Signal processing ---
MIN_CONTACT_IR = 900            # avg IR below this => finger not on the sensor
BANDPASS_CUTOFF = [0.7, 3.5]    # Hz bandpass (~42-210 BPM) to clean the PPG
BANDPASS_ORDER = 3

# --- Sleep-stage detection ---
BASELINE_READINGS = 60          # readings averaged to set the resting baseline
DEEP_SLEEP_DROP_PCT = 20        # % BPM drop from baseline => deep sleep (wake!)
FALLING_ASLEEP_DROP_PCT = 10    # % BPM drop from baseline => falling asleep
