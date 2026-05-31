#pragma once
#include <Arduino.h>

// Joystick input -> debounced button events for menu navigation.

enum class Button {
  None,
  Up,
  Down,
  Left,
  Right,
  A,
  B,
  C,
  D
};

void inputsInit();   // configure joystick ADC pins/resolution

// Sample the joystick (with crosstalk filtering + debounce) and return the
// resulting button event, or Button::None.
Button pollJoystick(uint32_t nowMs);
