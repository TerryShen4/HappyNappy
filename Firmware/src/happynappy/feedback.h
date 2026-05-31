#pragma once
#include <Arduino.h>

// Vibration motor + speaker output. The vibration pattern state is owned here.

void feedbackInit();   // configure motor pin + speaker PWM channel

void StartVibrationPattern(uint8_t pattern, bool repeat);
void StopVibration();
void UpdateVibration(uint32_t nowMs);   // call every loop to advance the pattern
