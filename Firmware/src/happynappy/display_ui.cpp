#include "display_ui.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "app_state.h"
#include "config.h"

namespace {

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void DrawCenteredText(const char *text, int y, uint8_t size) {
  display.setTextSize(size);
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  int x = (SCREEN_WIDTH - w) / 2;
  display.setCursor(x, y);
  display.println(text);
}

}  // namespace

void displayInit() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("SSD1306 allocation failed");
  } else {
    Serial.println("OLED display initialized!");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Initializing...");
    display.display();
  }
}

void renderScreen(const RtcDateTime &now, uint32_t nowMs, bool timeValid) {
  display.clearDisplay();

  if (currentScreen == Screen::Dashboard) {
    if (timeValid) {
      display.setTextSize(2);
      char timeStr[10];
      snprintf(timeStr, sizeof(timeStr), "%02u:%02u:%02u",
               now.Hour(), now.Minute(), now.Second());
      DrawCenteredText(timeStr, 8, 2);

      display.setTextSize(1);
      char dateStr[12];
      snprintf(dateStr, sizeof(dateStr), "%04u-%02u-%02u",
               now.Year(), now.Month(), now.Day());
      DrawCenteredText(dateStr, 30, 1);
    }

    display.setTextSize(1);

  } else if (currentScreen == Screen::MainMenu) {
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Main Menu");
    display.setCursor(0, 16);
    display.print(mainMenuIndex == 0 ? "> " : "  ");
    display.print("Alarm: ");
    display.println(alarmEnabled ? "On" : "Off");
    display.print(mainMenuIndex == 1 ? "> " : "  ");
    display.println("View Alarm");
    display.print(mainMenuIndex == 2 ? "> " : "  ");
    display.println("Set Alarm");
    display.print(mainMenuIndex == 3 ? "> " : "  ");
    display.println("Set Alarm Type");
    display.print(mainMenuIndex == 4 ? "> " : "  ");
    display.println("System Settings");
  } else if (currentScreen == Screen::ViewAlarm) {
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Current Alarm");
    display.setCursor(0, 16);
    char alarmStr[12];
    snprintf(alarmStr, sizeof(alarmStr), "%02u:%02u",
             alarmTime.Hour(), alarmTime.Minute());
    DrawCenteredText(alarmStr, 25, 2);
    const char *typeStr;
    if (alarmType == 0) {
      typeStr = "Normal Alarm";
    } else if (alarmType == 1) {
      typeStr = "Max Sleep Mode";
    } else {
      typeStr = "Smart Sleep Mode";
    }
    DrawCenteredText(typeStr, 45, 1);
  } else if (currentScreen == Screen::SetAlarm) {
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Set Alarm Time");
    display.setCursor(0, 16);
    char dateStr[12];
    snprintf(dateStr, sizeof(dateStr), "%04u-%02u-%02u",
             alarmYear, alarmMonth, alarmDay);
    display.println(dateStr);
    display.setCursor(0, 26);
    char timeStr[10];
    snprintf(timeStr, sizeof(timeStr), "%02u:%02u", alarmHour, alarmMinute);
    display.println(timeStr);
    display.setCursor(0, 36);
    display.print("Now: ");
    char nowStr[10];
    snprintf(nowStr, sizeof(nowStr), "%02u:%02u",
             now.Hour(), now.Minute());
    display.println(nowStr);
    bool blinkOn = ((nowMs / 400) % 2) == 0;
    if (blinkOn) {
      const int charW = 6;
      const int charH = 8;
      int x = 0;
      int y = 0;
      int w = 0;
      int h = charH + 2;
      if (setAlarmFieldIndex == 0) {
        x = 0; y = 15; w = 4 * charW;
      } else if (setAlarmFieldIndex == 1) {
        x = 5 * charW; y = 15; w = 2 * charW;
      } else if (setAlarmFieldIndex == 2) {
        x = 8 * charW; y = 15; w = 2 * charW;
      } else if (setAlarmFieldIndex == 3) {
        x = 0; y = 25; w = 2 * charW;
      } else {
        x = 3 * charW; y = 25; w = 2 * charW;
      }
      display.drawRect(x - 1, y - 1, w + 2, h, SSD1306_WHITE);
    }
  } else if (currentScreen == Screen::SetAlarmType) {
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Set Alarm Type");
    display.setCursor(0, 16);
    display.print(setAlarmTypeIndex == 0 ? "> " : "  ");
    display.println("Normal Alarm");
    display.print(setAlarmTypeIndex == 1 ? "> " : "  ");
    display.println("Max Sleep Mode");
    display.print(setAlarmTypeIndex == 2 ? "> " : "  ");
    display.println("Smart Sleep Mode");

    if (showAlarmTypeInfo) {
      display.setCursor(0, 45);
      if (setAlarmTypeIndex == 0) {
        display.println("Sounds at set");
        display.println("time only");
      } else if (setAlarmTypeIndex == 1) {
        display.println("Wakes before deep");
        display.println("sleep (< 1 cycle)");
      } else {
        display.println("Wakes before deep");
        display.println("sleep or timer end");
      }
    }
  } else if (currentScreen == Screen::SmartAlarm) {
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Smart Alarm");
    display.setCursor(0, 16);
    display.print(smartAlarmMenuIndex == 0 ? "> " : "  ");
    display.println("Max Sleep");
    display.print(smartAlarmMenuIndex == 1 ? "> " : "  ");
    display.println("Smart Sleep");

    if (showSmartAlarmInfo) {
      display.setCursor(0, 35);
      if (smartAlarmMenuIndex == 0) {
        display.println("Wakes before deep");
        display.println("sleep (< 1 cycle)");
      } else {
        display.println("Wakes before deep");
        display.println("sleep or timer end");
      }
    }
  } else if (currentScreen == Screen::SystemSettingsMenu) {
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("System Settings");
    display.setCursor(0, 16);
    display.print(systemMenuIndex == 0 ? "> " : "  ");
    display.println("Select Tone");
    display.print(systemMenuIndex == 1 ? "> " : "  ");
    display.println("Alarm Output");
    display.print(systemMenuIndex == 2 ? "> " : "  ");
    display.println("Test Output");
  } else if (currentScreen == Screen::AlarmOutput) {
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Alarm Output");
    display.setCursor(0, 16);
    display.print(alarmOutputIndex == 0 ? "> " : "  ");
    display.println("Vibration");
    display.print(alarmOutputIndex == 1 ? "> " : "  ");
    display.println("Speaker");
    display.print(alarmOutputIndex == 2 ? "> " : "  ");
    display.println("Both");
  } else if (currentScreen == Screen::TestAlarmOutput) {
    display.setTextSize(1);
    DrawCenteredText("Test Alarm Output", 0, 1);
    const char *outputStr = "Testing: Vibration";
    if (alarmOutput == 1) {
      outputStr = "Testing: Speaker";
    } else if (alarmOutput == 2) {
      outputStr = "Testing: Both";
    }
    DrawCenteredText(outputStr, 20, 1);
    if (testAlarmPlaying) {
      DrawCenteredText("Playing...", 35, 1);
    } else {
      DrawCenteredText("Press Right to Test", 35, 1);
    }
  } else if (currentScreen == Screen::SystemTone) {
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Select Tone");
    display.setCursor(0, 16);
    display.print(alarmToneIndex == 0 ? "> " : "  ");
    display.println("Tone 1");
    display.print(alarmToneIndex == 1 ? "> " : "  ");
    display.println("Tone 2");
    display.print(alarmToneIndex == 2 ? "> " : "  ");
    display.println("Tone 3");
  } else if (currentScreen == Screen::Alert) {
    DrawCenteredText("ALARM!", 0, 2);
    display.setTextSize(1);
    display.setCursor(0, 34);
    display.print(alertMenuIndex == 0 ? "> " : "  ");
    display.println("Snooze");
    display.print(alertMenuIndex == 1 ? "> " : "  ");
    display.println("Dismiss");
  }

  display.display();
}
