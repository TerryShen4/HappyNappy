#include "app_state.h"

#include "config.h"
#include "feedback.h"

// --- Menu navigation state ---
Screen currentScreen = Screen::Dashboard;
uint8_t mainMenuIndex = 0;
uint8_t alertMenuIndex = 0;
uint8_t systemMenuIndex = 0;
uint8_t smartAlarmMenuIndex = 0;
uint8_t alarmOutputIndex = 0;
uint8_t alarmToneIndex = 0;
uint8_t setAlarmFieldIndex = 0;
uint8_t setAlarmTypeIndex = 0;
uint8_t lastTonePreview = 255;

// --- Alarm configuration ---
bool alarmEnabled = true;
uint16_t alarmYear = 2026;
uint8_t alarmMonth = 1;
uint8_t alarmDay = 1;
uint8_t alarmHour = kAlarmHour;
uint8_t alarmMinute = kAlarmMinute;
uint8_t alarmType = 0;
uint8_t alarmOutput = 0;
bool showSmartAlarmInfo = false;
bool showAlarmTypeInfo = false;
bool testAlarmPlaying = false;
uint32_t testAlarmStartMs = 0;

// --- Alarm scheduling ---
RtcDateTime alarmTime;
bool alarmTriggered = false;

void SetAlarmFromFields() {
  alarmTime = RtcDateTime(alarmYear, alarmMonth, alarmDay,
                          alarmHour, alarmMinute, kAlarmSecond);
  alarmTriggered = false;
}

void SetAlarmDefaultsFromNow(const RtcDateTime &now) {
  RtcDateTime next = RtcDateTime(now.Unix32Time() + 3600);
  alarmYear = 2026;
  alarmMonth = next.Month();
  alarmDay = next.Day();
  alarmHour = next.Hour();
  alarmMinute = next.Minute();
  SetAlarmFromFields();
}

void buzzMotor() {
  alarmTriggered = true;
  currentScreen = Screen::Alert;
  alertMenuIndex = 0;
  StartVibrationPattern(alarmToneIndex, true);
}
