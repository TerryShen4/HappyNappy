#pragma once
#include <Arduino.h>
#include <RtcUtility.h>  // defines countof(), needed by RtcDateTime.h
#include <RtcDateTime.h>

#include "inputs.h"

// Apply a button event to the current screen: navigation + menu actions.
// `now` is taken by value because RtcDateTime's comparison operators are
// non-const (they need a mutable left-hand operand).
void handleInput(Button button, RtcDateTime now, bool timeValid, uint32_t nowMs);
