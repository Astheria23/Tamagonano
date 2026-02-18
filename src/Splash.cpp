#include "Splash.h"

namespace {
constexpr int kBuzzerChannel = 0;
constexpr int kBuzzerResolution = 8;
constexpr int kBuzzerFreq = 1568; // G6
constexpr int kBuzzerTickMs = 40;
constexpr int kTextSize = 2;
constexpr const char *kTitle = "Tamagonano";
constexpr const char *kSubtitle = "Made with \x03 by Oiq";
constexpr int kHoldMs = 600;

int textWidthPx() {
  const int charWidth = 6 * kTextSize;
  return static_cast<int>(strlen(kTitle)) * charWidth;
}

void playTick() {
  ledcWriteTone(kBuzzerChannel, kBuzzerFreq);
  delay(kBuzzerTickMs);
  ledcWriteTone(kBuzzerChannel, 0);
}
} // namespace

void showSplash() {
  ledcSetup(kBuzzerChannel, kBuzzerFreq, kBuzzerResolution);
  ledcAttachPin(BUZZER_PIN, kBuzzerChannel);

  display.clearDisplay();
  display.setTextSize(kTextSize);
  display.setTextColor(WHITE);

  const int textWidth = textWidthPx();
  const int titleHeight = 8 * kTextSize;
  const int subtitleHeight = 8;
  const int totalHeight = titleHeight + 6 + subtitleHeight;
  const int x = (SCREEN_WIDTH - textWidth) / 2;
  const int y = (SCREEN_HEIGHT - totalHeight) / 2;

  for (size_t i = 1; i <= strlen(kTitle); ++i) {
    display.clearDisplay();
    display.setCursor(x, y);
    display.write(reinterpret_cast<const uint8_t *>(kTitle), i);
    display.setTextSize(1);
    display.setCursor((SCREEN_WIDTH - (6 * static_cast<int>(strlen(kSubtitle)))) / 2, y + titleHeight + 6);
    display.print(kSubtitle);
    display.setTextSize(kTextSize);
    display.display();
    playTick();
    delay(80);
  }

  // Blur effect by drawing offsets around the text.
  for (int blur = 0; blur < 4; ++blur) {
    display.clearDisplay();
    display.setTextSize(kTextSize);
    for (int dx = -1; dx <= 1; ++dx) {
      for (int dy = -1; dy <= 1; ++dy) {
        if (dx == 0 && dy == 0) continue;
        display.setCursor(x + dx, y + dy);
        display.print(kTitle);
      }
    }
    display.setCursor(x, y);
    display.print(kTitle);
    display.setTextSize(1);
    display.setCursor((SCREEN_WIDTH - (6 * static_cast<int>(strlen(kSubtitle)))) / 2, y + titleHeight + 6);
    display.print(kSubtitle);
    display.display();
    delay(120);
  }

  delay(kHoldMs);
  ledcDetachPin(BUZZER_PIN);
}
