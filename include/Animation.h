#ifndef ANIMATION_H
#define ANIMATION_H

#include <Arduino.h>
#include "Config.h"

extern unsigned long nextIdleAction;

void handleIdleLogic() {
  unsigned long now = millis();
  if (now > nextIdleAction) {
    tarX = random(-22, 22); 
    tarY = random(-12, 10);
    tarS = constrain(90 + (tarX * 1.8), 40, 140); 
    nextIdleAction = now + random(1500, 4500);
  }
  if (random(1000) > 985) tarH = 2; 
}

void updatePhysics(Servo &s) {
  // LERP Animasi
  curX += (tarX - curX) * 0.12;
  curY += (tarY - curY) * 0.12;
  curW += (tarW - curW) * 0.15;
  curH += (tarH - curH) * 0.25;
  curS += (tarS - curS) * 0.08;

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