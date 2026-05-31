#pragma once
#include <Arduino.h>

// Shared compile-time configuration for the HappyNappy firmware.
// Kept in one place so pins, credentials, and timing are easy to find/adjust.

// --- I2C ---
#define SDA_PIN 21
#define SCL_PIN 22

// --- OLED display ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// --- WiFi / MQTT ---
static const char *kWifiSsid = "WIFI-FF44";
static const char *kWifiPass = "canal4165artist";
static const char *kMqttHost = "10.0.0.228"; // WIFI-FF44 broker host (this PC)
static const uint16_t kMqttPort = 1883;
static const char *kMqttTopic = "sensors/max30102/data";

// --- Sensor sampling ---
static const uint16_t kSampleRateHz = 50;            // 50 Hz => 20 ms per sample
static const uint16_t kWindowSeconds = 10;           // collect for 10 seconds
static const uint16_t kSamplesPerWindow = kSampleRateHz * kWindowSeconds; // 500
static const uint16_t kSampleDelayMs = 1000 / kSampleRateHz;

// --- DS1302 RTC pins ---
static const uint8_t kClkPin = 18;
static const uint8_t kDatPin = 19;
static const uint8_t kRstPin = 23;

// --- Alarm defaults (24-hour) ---
static const uint8_t kAlarmHour = 7;
static const uint8_t kAlarmMinute = 30;
static const uint8_t kAlarmSecond = 0;
static const uint16_t kSnoozeMinutes = 5;

// --- Vibration motor + speaker ---
static const uint8_t kVibePin = 25;
static const uint8_t kSpeakerPin = 26;
static const uint8_t kSpeakerChannel = 0;
static const uint16_t kSpeakerFreq = 2000;

// --- Joystick (ADC1 channels; ADC2 unavailable while WiFi is active) ---
static const int kJoystickPinX = 32;  // ADC1_CH4
static const int kJoystickPinY = 35;  // ADC1_CH5
