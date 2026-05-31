#pragma once
#include <Arduino.h>

// WiFi + MQTT transport. Publishes sensor windows and receives wake/control
// messages from the backend.

// Set true when the backend sends STOP_SAMPLING (stop collecting/sending data).
extern bool stopSampling;

void networkSetup();            // connect WiFi + configure the MQTT client
void networkEnsureConnected();  // (re)connect to the broker + service MQTT

// Publish one window of IR samples as JSON. Returns true on success.
bool publishSamples(const long *samples, uint16_t count);
