#pragma once
#include <Arduino.h>

// Bluetooth Classic (SPP) transport. Streams sensor windows to the paired
// laptop and receives wake/control commands back over the same link.

// Set true when the backend sends STOP_SAMPLING (stop collecting/sending data).
extern bool stopSampling;

void bluetoothSetup();          // start the SPP device (advertised as kBtName)
void bluetoothPoll();           // service incoming bytes / dispatch commands
bool bluetoothHasClient();      // true once the laptop is connected over SPP

// Send one window of IR samples as a JSON line. Returns true on success.
bool publishSamples(const long *samples, uint16_t count);
