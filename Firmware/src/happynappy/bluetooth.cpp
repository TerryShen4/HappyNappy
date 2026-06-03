#include "bluetooth.h"

#include "BluetoothSerial.h"

#include "app_state.h"
#include "config.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled. Run `make menuconfig` to enable it (it is on by \
default for the esp32dev Arduino core).
#endif

bool stopSampling = false;

namespace {

BluetoothSerial SerialBT;
String rxLine;  // accumulates one incoming command line until '\n'

// Act on a complete command line from the backend. Mirrors the old MQTT
// callback: a wake alert buzzes the motor; a stop command halts streaming.
void handleCommand(const String &cmd) {
  Serial.print("\nReceived command: ");
  Serial.println(cmd);

  if (cmd == "WAKE_UP") {
    Serial.println("\n=====================================");
    Serial.println("WAKE UP ALERT!");
    Serial.println("=====================================");
    buzzMotor();  // Activate vibration motor
  } else if (cmd == "STOP_SAMPLING") {
    stopSampling = true;
    Serial.println("Received STOP command - stopping data collection");
  }
}

}  // namespace

void bluetoothSetup() {
  SerialBT.begin(kBtName);  // non-blocking: device is discoverable immediately
  Serial.print("Bluetooth started. Pair with '");
  Serial.print(kBtName);
  Serial.println("' from the laptop, then open its outgoing COM port.");
}

void bluetoothPoll() {
  while (SerialBT.available()) {
    char c = (char)SerialBT.read();
    if (c == '\n' || c == '\r') {
      if (rxLine.length() > 0) {
        handleCommand(rxLine);
        rxLine = "";
      }
    } else {
      rxLine += c;
    }
  }
}

bool bluetoothHasClient() {
  return SerialBT.hasClient();
}

bool publishSamples(const long *samples, uint16_t count) {
  if (!SerialBT.hasClient()) {
    return false;
  }

  String payload;
  payload.reserve(3600);
  payload += "{\"ir\":[";
  for (uint16_t i = 0; i < count; ++i) {
    payload += String(samples[i]);
    if (i + 1 < count) {
      payload += ',';
    }
  }
  payload += "],\"sample_rate\":";
  payload += String(kSampleRateHz);
  payload += '}';

  // println() appends '\n' so the PC reads exactly one window per line.
  return SerialBT.println(payload) > 0;
}
