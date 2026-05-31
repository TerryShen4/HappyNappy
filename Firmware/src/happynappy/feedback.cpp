#include "feedback.h"

#include "config.h"

namespace {

// Vibration pattern state (private to this module).
struct VibeState {
  bool active = false;
  bool repeat = false;
  uint8_t pattern = 0;
  uint8_t step = 0;
  uint32_t nextMs = 0;
  bool on = false;
};

VibeState vibe;

}  // namespace

void feedbackInit() {
  pinMode(kVibePin, OUTPUT);
  digitalWrite(kVibePin, LOW);

  ledcSetup(kSpeakerChannel, kSpeakerFreq, 8);
  ledcAttachPin(kSpeakerPin, kSpeakerChannel);
  ledcWriteTone(kSpeakerChannel, 0);
}

void StartVibrationPattern(uint8_t pattern, bool repeat) {
  vibe.active = true;
  vibe.repeat = repeat;
  vibe.pattern = pattern;
  vibe.step = 0;
  vibe.on = true;
  vibe.nextMs = millis();
  digitalWrite(kVibePin, HIGH);
}

void StopVibration() {
  vibe.active = false;
  digitalWrite(kVibePin, LOW);
}

void UpdateVibration(uint32_t nowMs) {
  if (!vibe.active) return;

  const uint16_t pattern0[] = {200, 200, 200, 800};
  const uint16_t pattern1[] = {400, 200, 400, 200, 400, 800};
  const uint16_t pattern2[] = {400, 400, 400, 800};

  const uint16_t *pattern = pattern0;
  uint8_t length = 4;
  if (vibe.pattern == 1) {
    pattern = pattern1;
    length = 6;
  } else if (vibe.pattern == 2) {
    pattern = pattern2;
    length = 2;
  }

  if (nowMs < vibe.nextMs) return;

  vibe.on = !vibe.on;
  digitalWrite(kVibePin, vibe.on ? HIGH : LOW);
  uint16_t duration = pattern[vibe.step];
  vibe.nextMs = nowMs + duration;

  vibe.step++;
  if (vibe.step >= length) {
    if (vibe.repeat) {
      vibe.step = 0;
      vibe.on = true;
      digitalWrite(kVibePin, HIGH);
      vibe.nextMs = nowMs + pattern[0];
    } else {
      StopVibration();
    }
  }
}
