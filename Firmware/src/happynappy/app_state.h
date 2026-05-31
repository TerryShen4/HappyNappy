#pragma once
#include <Arduino.h>
#include <RtcUtility.h>  // defines countof(), needed by RtcDateTime.h
#include <RtcDateTime.h>

// Shared UI / alarm state for the menu system. These are plain globals (as in
// the original firmware) but declared here so the display and menu modules
// share one source of truth instead of scattering the state.

enum class Screen {
  Dashboard,
  MainMenu,
  ViewAlarm,
  SetAlarm,
  SetAlarmType,
  SmartAlarm,
  SmartAlarmInfo,
  SystemSettingsMenu,
  AlarmOutput,
  SystemTone,
  TestAlarmOutput,
  Alert
};

// --- Menu navigation state ---
extern Screen currentScreen;
extern uint8_t mainMenuIndex;
extern uint8_t alertMenuIndex;
extern uint8_t systemMenuIndex;
extern uint8_t smartAlarmMenuIndex;
extern uint8_t alarmOutputIndex;
extern uint8_t alarmToneIndex;
extern uint8_t setAlarmFieldIndex;
extern uint8_t setAlarmTypeIndex;
extern uint8_t lastTonePreview;

// --- Alarm configuration ---
extern bool alarmEnabled;
extern uint16_t alarmYear;
extern uint8_t alarmMonth;
extern uint8_t alarmDay;
extern uint8_t alarmHour;
extern uint8_t alarmMinute;
extern uint8_t alarmType;        // 0 = Normal, 1 = Max Sleep, 2 = Smart Sleep
extern uint8_t alarmOutput;      // 0 = Vibration, 1 = Speaker, 2 = Both
extern bool showSmartAlarmInfo;
extern bool showAlarmTypeInfo;
extern bool testAlarmPlaying;
extern uint32_t testAlarmStartMs;

// --- Alarm scheduling ---
extern RtcDateTime alarmTime;
extern bool alarmTriggered;

// Apply the alarmYear/Month/Day/Hour/Minute fields to alarmTime.
void SetAlarmFromFields();
// Seed the alarm fields to one hour from `now`.
void SetAlarmDefaultsFromNow(const RtcDateTime &now);
// Enter the alarm screen and start buzzing (called when a wake alert arrives).
void buzzMotor();
