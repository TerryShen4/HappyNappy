#include "inputs.h"

#include <string>

#include "config.h"

using std::string;

namespace {

int joystickReadingX;
int joystickReadingY;

// Map a smoothed joystick position to a direction string.
std::string joystickReading(int x, int y) {
  string pressed = "nopress";

  // Only one direction at a time, check in priority order
  if (x > 4000) {
    pressed = "left";
  } else if (x < 50) {
    pressed = "right";
  } else if (y > 4000) {
    pressed = "up";
  } else if (y < 50) {
    pressed = "down";
  }

  return pressed;
}

Button toButton(const std::string &value) {
  if (value == "up") return Button::Up;
  if (value == "down") return Button::Down;
  if (value == "left") return Button::Left;
  if (value == "right") return Button::Right;
  if (value == "a") return Button::A;
  if (value == "b") return Button::B;
  if (value == "c") return Button::C;
  if (value == "d") return Button::D;
  return Button::None;
}

// Debounce: only emit an event when the direction changes and enough time
// has passed since the last change.
Button readJoystickEvent(uint32_t nowMs) {
  static std::string lastValue = "nopress";
  static uint32_t lastChangeMs = 0;

  std::string valueRead = joystickReading(joystickReadingX, joystickReadingY);

  if (valueRead == "nopress") {
    lastValue = valueRead;
    return Button::None;
  }

  if (valueRead != lastValue && (nowMs - lastChangeMs) > 180) {
    lastValue = valueRead;
    lastChangeMs = nowMs;
    return toButton(valueRead);
  }

  return Button::None;
}

}  // namespace

void inputsInit() {
  pinMode(kJoystickPinX, INPUT);
  pinMode(kJoystickPinY, INPUT);

  // Configure ADC for joystick pins
  analogSetAttenuation(ADC_11db);  // Full 0-3.3V range
  analogReadResolution(12);         // 12-bit resolution (0-4095)
}

Button pollJoystick(uint32_t nowMs) {
  // Read ADC with multiple samples and validation
  int x_sum = 0;
  int y_sum = 0;
  const int samples = 8;

  for (int i = 0; i < samples; i++) {
    // Read X axis with settling
    analogRead(kJoystickPinX);  // Dummy read
    delayMicroseconds(250);
    x_sum += analogRead(kJoystickPinX);
    delayMicroseconds(100);

    // Read Y axis with settling
    analogRead(kJoystickPinY);  // Dummy read
    delayMicroseconds(250);
    y_sum += analogRead(kJoystickPinY);
    delayMicroseconds(100);
  }

  int raw_x = x_sum / samples;
  int raw_y = y_sum / samples;

  // Filter out ADC crosstalk - when one axis is at extreme, other gets pulled up
  const int nearMax = 3800;
  const int crossTalkThreshold = 3000;  // If other axis also above this, likely crosstalk

  // If X is at max but Y is suspiciously high (crosstalk), correct Y
  if (raw_x > nearMax && raw_y > crossTalkThreshold && raw_y < nearMax) {
    raw_y = 2048;  // Reset Y to center
  }
  // If Y is at max but X is suspiciously high (crosstalk), correct X
  else if (raw_y > nearMax && raw_x > crossTalkThreshold && raw_x < nearMax) {
    raw_x = 2048;  // Reset X to center
  }
  // If both are at max, keep the more extreme one
  else if (raw_x > nearMax && raw_y > nearMax) {
    if (raw_x > raw_y) {
      raw_y = 2048;
    } else {
      raw_x = 2048;
    }
  }

  joystickReadingX = raw_x;
  joystickReadingY = raw_y;

  return readJoystickEvent(nowMs);
}
