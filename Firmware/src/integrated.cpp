// HappyNappy integrated firmware -- top-level orchestration.
//
// The subsystems live in src/happynappy/:
//   config.h     -- pins, credentials, timing constants
//   feedback.*   -- vibration motor + speaker
//   inputs.*     -- joystick -> Button events
//   app_state.*  -- menu/alarm state + Screen enum
//   display_ui.* -- OLED rendering
//   menu.*       -- button handling / menu navigation
//   bluetooth.*  -- Bluetooth SPP (stream sensor windows, receive wake alerts)
//
// This file owns the RTC and the heart-rate sensor, and wires everything
// together in setup()/loop().

#include <Arduino.h>
#include <Wire.h>
#include <MAX30105.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>

#include "happynappy/app_state.h"
#include "happynappy/config.h"
#include "happynappy/display_ui.h"
#include "happynappy/feedback.h"
#include "happynappy/inputs.h"
#include "happynappy/menu.h"
#include "happynappy/bluetooth.h"

ThreeWire rtcWire(kDatPin, kClkPin, kRstPin);  // DAT, CLK, RST
RtcDS1302<ThreeWire> rtc(rtcWire);

MAX30105 particleSensor;

static void PrintDateTime(const RtcDateTime &dt) {
  char buf[32];
  snprintf(buf, sizeof(buf), "%04u-%02u-%02u %02u:%02u:%02u",
           dt.Year(), dt.Month(), dt.Day(),
           dt.Hour(), dt.Minute(), dt.Second());
  Serial.println(buf);
}

void setup() {
  Serial.begin(115200);
  delay(200);

  bluetoothSetup();  // Bluetooth SPP (non-blocking; OLED comes up right away)

  Wire.begin(SDA_PIN, SCL_PIN);

  displayInit();   // OLED
  feedbackInit();  // vibration motor + speaker
  inputsInit();    // joystick ADC

  // Heart-rate sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println("MAX30102 was not found. Please check wiring/power.");
    while (1);
  }
  Serial.println("MAX30102 Sensor Initialized Successfully!");
  particleSensor.setup();  // Default configuration
  particleSensor.setPulseAmplitudeRed(0x0A);   // Turn on Red LED
  particleSensor.setPulseAmplitudeGreen(0);    // Turn off Green LED
  particleSensor.setPulseAmplitudeIR(0x0A);    // Turn on IR LED

  // Real-time clock
  rtc.Begin();
  if (!rtc.GetIsRunning()) {
    rtc.SetIsRunning(true);
  }
  if (!rtc.IsDateTimeValid()) {
    // RTC lost confidence: set to compile time
    rtc.SetDateTime(RtcDateTime(__DATE__, __TIME__));
  }

  RtcDateTime now = rtc.GetDateTime();
  if (rtc.IsDateTimeValid()) {
    SetAlarmDefaultsFromNow(now);
  }

  Serial.println("RTC initialized. Current time:");
  PrintDateTime(now);
}

void loop() {
  static uint32_t lastDisplayMs = 0;
  static uint32_t lastTimeMs = 0;
  const uint32_t nowMs = millis();

  UpdateVibration(nowMs);
  Button button = pollJoystick(nowMs);

  RtcDateTime now = rtc.GetDateTime();
  const bool timeValid = rtc.IsDateTimeValid();

  // Scheduled alarm fired?
  if (timeValid && alarmEnabled && !alarmTriggered && now >= alarmTime) {
    alarmTriggered = true;
    currentScreen = Screen::Alert;
    alertMenuIndex = 0;
    StartVibrationPattern(alarmToneIndex, true);
  }

  handleInput(button, now, timeValid, nowMs);

  bluetoothPoll();  // service incoming wake/control commands

  // Stop collecting/sending once the backend has triggered a wake alert.
  if (stopSampling) {
    delay(100);
    return;
  }

  // Collect samples for HeartPy analysis, sending one full window at a time.
  static uint16_t sampleIndex = 0;
  static long irSamples[kSamplesPerWindow];

  irSamples[sampleIndex++] = particleSensor.getIR();

  if (sampleIndex >= kSamplesPerWindow) {
    if (!bluetoothHasClient()) {
      Serial.println("Window ready, waiting for Bluetooth connection...");
    } else if (publishSamples(irSamples, kSamplesPerWindow)) {
      Serial.print("Sent ");
      Serial.print(kSamplesPerWindow);
      Serial.println("-sample window successfully");
    } else {
      Serial.println("Send FAILED");
    }
    sampleIndex = 0;
  }

  delay(kSampleDelayMs);

  // Redraw the screen (slower on the dashboard, snappier in menus).
  uint32_t displayIntervalMs = (currentScreen == Screen::Dashboard) ? 1000 : 200;
  if (nowMs - lastDisplayMs >= displayIntervalMs) {
    lastDisplayMs = nowMs;
    renderScreen(now, nowMs, timeValid);
  }

  // Heartbeat log + alarm check every 5 seconds.
  if (nowMs - lastTimeMs >= 5000) {
    lastTimeMs = nowMs;

    if (!timeValid) {
      Serial.println("RTC invalid time");
      return;
    }

    PrintDateTime(now);

    if (!alarmTriggered && now >= alarmTime) {
      alarmTriggered = true;
      Serial.println("ALARM: time reached");
    }
  }
}
