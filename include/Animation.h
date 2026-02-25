#ifndef ANIMATION_H
#define ANIMATION_H

#include <Arduino.h>
#include "Config.h"

extern unsigned long nextIdleAction;
extern unsigned long nextBlinkTime;
extern unsigned long blinkReleaseTime;

static inline void playHum() {
  static bool buzzerInit = false;
  constexpr int kHumChannel = 4;
  constexpr int kHumResolution = 8;
  constexpr int kHumFreq = 220; // soft hum
  if (!buzzerInit) {
    ledcSetup(kHumChannel, kHumFreq, kHumResolution);
    ledcAttachPin(BUZZER_PIN, kHumChannel);
    buzzerInit = true;
  }
  ledcWriteTone(kHumChannel, kHumFreq);
  delay(120);
  ledcWriteTone(kHumChannel, 0);
}

void handleIdleLogic() {
  unsigned long now = millis();
  static unsigned long lastHumTime = 0;
  if (now > nextIdleAction) {
    const bool lookAway = random(1000) < 140;
    if (lookAway) {
      tarX = random(-22, 22); 
      tarY = random(-12, 10);
      if (now - lastHumTime > 2000) {
        playHum();
        lastHumTime = now;
      }
    } else {
      tarX = random(-4, 5);
      tarY = random(-3, 4);
    }
    tarS = constrain(90 + (tarX * 1.8), 40, 140); 
    nextIdleAction = now + random(2400, 7000);
  }

  if (now > nextBlinkTime) {
    tarBlink = 1.0f;
    blinkReleaseTime = now + random(110, 180);
    nextBlinkTime = now + random(2200, 6000);
  }
  if (tarBlink > 0.0f && now > blinkReleaseTime) {
    tarBlink = 0.0f;
  }
}

void updatePhysics(Servo &s) {
  // LERP Animasi
  curX += (tarX - curX) * 0.12;
  curY += (tarY - curY) * 0.12;
  curW += (tarW - curW) * 0.15;
  curH += (tarH - curH) * 0.25;
  curS += (tarS - curS) * 0.08;
  curBlink += (tarBlink - curBlink) * 0.35;

  unsigned long now = millis();
  // Gunakan variabel global yang sudah extern
  if (now - lastServoUpdate >= 50) {
    int targetInt = (int)curS;
    if (abs(lastWrittenServo - targetInt) >= 1) {
      s.write(targetInt);
      lastWrittenServo = targetInt;
      lastServoUpdate = now;
    }
  }
}

#endif