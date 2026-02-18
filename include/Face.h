#ifndef FACE_H
#define FACE_H

#include "Config.h"

void drawNavbar(int batteryPct, bool wifiConnected) {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  
  int xW = 100; 
  if (wifiConnected) {
    display.fillRect(xW, 6, 2, 2, WHITE);
    display.fillRect(xW + 3, 4, 2, 4, WHITE);
    display.fillRect(xW + 6, 2, 2, 6, WHITE);
  } else {
    display.drawPixel(xW + 3, 6, WHITE); 
  }

  // 2. Icon Baterai (Diperkecil)
  int xB = 112; // Posisi X Batre
  display.drawRect(xB, 2, 12, 6, WHITE); // Body lebih kecil
  display.fillRect(xB + 12, 4, 1, 2, WHITE); // Kepala batre
  
  int barWidth = map(batteryPct, 0, 100, 0, 10);
  display.fillRect(xB + 1, 3, barWidth, 4, WHITE);
}

void renderFace() {
  int xL = 40 + curX;
  int xR = 88 + curX;
  int y = 35 + curY; // Sedikit naik karena navbar sudah tipis tanpa teks

  // Mata tetap pakai RoundRect biar estetik
  display.fillRoundRect(xL - curW/2, y - curH/2, curW, curH, 4, WHITE);
  display.fillRoundRect(xR - curW/2, y - curH/2, curW, curH, 4, WHITE);

  // Eyelids for blink (draw black masks from top/bottom)
  float blink = constrain(curBlink, 0.0f, 1.0f);
  if (blink > 0.01f) {
    int lid = static_cast<int>((curH * blink) / 2.0f);
    int eyeW = static_cast<int>(curW);
    int eyeH = static_cast<int>(curH);
    display.fillRect(xL - eyeW/2, y - eyeH/2, eyeW, lid, BLACK);
    display.fillRect(xL - eyeW/2, y + eyeH/2 - lid, eyeW, lid, BLACK);
    display.fillRect(xR - eyeW/2, y - eyeH/2, eyeW, lid, BLACK);
    display.fillRect(xR - eyeW/2, y + eyeH/2 - lid, eyeW, lid, BLACK);
  }

  // Mouth intentionally omitted for a cleaner expression
}

#endif