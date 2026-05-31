#include "menu.h"

#include "app_state.h"
#include "config.h"
#include "feedback.h"

void handleInput(Button button, RtcDateTime now, bool timeValid, uint32_t nowMs) {
  switch (currentScreen) {
    case Screen::Dashboard:
      if (button == Button::Right) {
        currentScreen = Screen::MainMenu;
        mainMenuIndex = 0;
      }
      break;
    case Screen::MainMenu:
      if (button == Button::Up && mainMenuIndex > 0) {
        mainMenuIndex--;
      } else if (button == Button::Down && mainMenuIndex < 4) {
        mainMenuIndex++;
      } else if (button == Button::Right) {
        if (mainMenuIndex == 0) {
          alarmEnabled = !alarmEnabled;
          if (!alarmEnabled) {
            alarmTriggered = false;
            StopVibration();
          }
        } else if (mainMenuIndex == 1) {
          currentScreen = Screen::ViewAlarm;
        } else if (mainMenuIndex == 2) {
          if (timeValid) {
            SetAlarmDefaultsFromNow(now);
          }
          setAlarmFieldIndex = 0;
          currentScreen = Screen::SetAlarm;
        } else if (mainMenuIndex == 3) {
          setAlarmTypeIndex = alarmType;
          currentScreen = Screen::SetAlarmType;
        } else if (mainMenuIndex == 4) {
          systemMenuIndex = 0;
          currentScreen = Screen::SystemSettingsMenu;
        }
      } else if (button == Button::Left) {
        currentScreen = Screen::Dashboard;
      }
      break;
    case Screen::ViewAlarm:
      if (button == Button::Left) {
        currentScreen = Screen::MainMenu;
      }
      break;
    case Screen::SetAlarm:
      if (button == Button::Up) {
        if (setAlarmFieldIndex == 0) {
          alarmYear = (alarmYear < 2099) ? alarmYear + 1 : 2000;
        } else if (setAlarmFieldIndex == 1) {
          alarmMonth = (alarmMonth % 12) + 1;
        } else if (setAlarmFieldIndex == 2) {
          alarmDay = (alarmDay % 31) + 1;
        } else if (setAlarmFieldIndex == 3) {
          alarmHour = (alarmHour + 1) % 24;
        } else {
          alarmMinute = (alarmMinute + 1) % 60;
        }
      } else if (button == Button::Down) {
        if (setAlarmFieldIndex == 0) {
          alarmYear = (alarmYear > 2000) ? alarmYear - 1 : 2099;
        } else if (setAlarmFieldIndex == 1) {
          alarmMonth = (alarmMonth == 1) ? 12 : alarmMonth - 1;
        } else if (setAlarmFieldIndex == 2) {
          alarmDay = (alarmDay == 1) ? 31 : alarmDay - 1;
        } else if (setAlarmFieldIndex == 3) {
          alarmHour = (alarmHour == 0) ? 23 : alarmHour - 1;
        } else {
          alarmMinute = (alarmMinute == 0) ? 59 : alarmMinute - 1;
        }
      } else if (button == Button::Right) {
        if (setAlarmFieldIndex < 4) {
          setAlarmFieldIndex++;
        } else {
          SetAlarmFromFields();
          currentScreen = Screen::MainMenu;
        }
      } else if (button == Button::Left) {
        SetAlarmFromFields();
        currentScreen = Screen::MainMenu;
      }
      break;
    case Screen::SetAlarmType:
      if (button == Button::Up && setAlarmTypeIndex > 0) {
        setAlarmTypeIndex--;
        showAlarmTypeInfo = false;
      } else if (button == Button::Down && setAlarmTypeIndex < 2) {
        setAlarmTypeIndex++;
        showAlarmTypeInfo = false;
      } else if (button == Button::Right) {
        if (showAlarmTypeInfo) {
          alarmType = setAlarmTypeIndex;
          currentScreen = Screen::MainMenu;
        } else {
          showAlarmTypeInfo = true;
        }
      } else if (button == Button::Left) {
        if (showAlarmTypeInfo) {
          showAlarmTypeInfo = false;
        } else {
          currentScreen = Screen::MainMenu;
        }
        currentScreen = Screen::MainMenu;
      }
      break;
    case Screen::SmartAlarm:
      if (button == Button::Up && smartAlarmMenuIndex > 0) {
        smartAlarmMenuIndex--;
        showSmartAlarmInfo = false;
      } else if (button == Button::Down && smartAlarmMenuIndex < 1) {
        smartAlarmMenuIndex++;
        showSmartAlarmInfo = false;
      } else if (button == Button::Right) {
        if (showSmartAlarmInfo) {
          showSmartAlarmInfo = false;
        } else {
          showSmartAlarmInfo = true;
        }
      } else if (button == Button::Left) {
        if (showSmartAlarmInfo) {
          showSmartAlarmInfo = false;
        } else {
          currentScreen = Screen::MainMenu;
        }
      }
      break;
    case Screen::SmartAlarmInfo:
      if (button == Button::Left) {
        currentScreen = Screen::SmartAlarm;
      }
      break;
    case Screen::SystemSettingsMenu:
      if (button == Button::Up && systemMenuIndex > 0) {
        systemMenuIndex--;
      } else if (button == Button::Down && systemMenuIndex < 2) {
        systemMenuIndex++;
      } else if (button == Button::Right) {
        if (systemMenuIndex == 0) {
          currentScreen = Screen::SystemTone;
          lastTonePreview = 255;
        } else if (systemMenuIndex == 1) {
          currentScreen = Screen::AlarmOutput;
          alarmOutputIndex = alarmOutput;
        } else if (systemMenuIndex == 2) {
          currentScreen = Screen::TestAlarmOutput;
          testAlarmPlaying = false;
        }
      } else if (button == Button::Left) {
        currentScreen = Screen::MainMenu;
      }
      break;
    case Screen::AlarmOutput:
      if (button == Button::Up && alarmOutputIndex > 0) {
        alarmOutputIndex--;
      } else if (button == Button::Down && alarmOutputIndex < 2) {
        alarmOutputIndex++;
      } else if (button == Button::Right) {
        alarmOutput = alarmOutputIndex;
        currentScreen = Screen::SystemSettingsMenu;
      } else if (button == Button::Left) {
        currentScreen = Screen::SystemSettingsMenu;
      }
      break;
    case Screen::SystemTone:
      if (button == Button::Up && alarmToneIndex > 0) {
        alarmToneIndex--;
      } else if (button == Button::Down && alarmToneIndex < 2) {
        alarmToneIndex++;
      } else if (button == Button::Right || button == Button::Left) {
        currentScreen = Screen::SystemSettingsMenu;
      }
      if (alarmToneIndex != lastTonePreview) {
        lastTonePreview = alarmToneIndex;
        StartVibrationPattern(alarmToneIndex, false);
      }
      break;
    case Screen::TestAlarmOutput:
      if (button == Button::Right && !testAlarmPlaying) {
        testAlarmPlaying = true;
        testAlarmStartMs = nowMs;
        if (alarmOutput == 0 || alarmOutput == 2) {
          StartVibrationPattern(alarmToneIndex, false);
        }
        if (alarmOutput == 1 || alarmOutput == 2) {
          ledcWriteTone(kSpeakerChannel, kSpeakerFreq);
        }
      } else if (button == Button::Left) {
        testAlarmPlaying = false;
        StopVibration();
        ledcWriteTone(kSpeakerChannel, 0);
        currentScreen = Screen::SystemSettingsMenu;
      }
      if (testAlarmPlaying && (alarmOutput == 1 || alarmOutput == 2)) {
        if (nowMs - testAlarmStartMs < 500) {
          ledcWriteTone(kSpeakerChannel, kSpeakerFreq);
        } else {
          ledcWriteTone(kSpeakerChannel, 0);
          testAlarmPlaying = false;
        }
      }
      break;
    case Screen::Alert:
      if (button == Button::Up || button == Button::Down) {
        alertMenuIndex = (alertMenuIndex == 0) ? 1 : 0;
      } else if (button == Button::Right) {
        if (alertMenuIndex == 0) {
          if (timeValid) {
            alarmTime = RtcDateTime(now.Unix32Time() + (kSnoozeMinutes * 60));
          }
          alarmTriggered = false;
          StopVibration();
          currentScreen = Screen::Dashboard;
        } else {
          alarmTriggered = false;
          StopVibration();
          if (timeValid && now >= alarmTime) {
            alarmTime = RtcDateTime(alarmTime.Unix32Time() + 86400);
          }
          currentScreen = Screen::Dashboard;
        }
      }
      break;
  }
}
