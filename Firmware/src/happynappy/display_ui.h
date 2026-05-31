#pragma once
#include <Arduino.h>
#include <RtcUtility.h>  // defines countof(), needed by RtcDateTime.h
#include <RtcDateTime.h>

// OLED rendering. Owns the SSD1306 display object.

void displayInit();   // begin the display and show a splash message

// Render the current screen (clears + draws + pushes to the panel).
void renderScreen(const RtcDateTime &now, uint32_t nowMs, bool timeValid);
