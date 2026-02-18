#include "Pages.h"

namespace {
void drawCenteredText(const char *text, int16_t y, uint8_t textSize) {
  display.setTextSize(textSize);
  display.setTextColor(WHITE);
  int16_t x = (SCREEN_WIDTH - (6 * textSize * strlen(text))) / 2;
  display.setCursor(x, y);
  display.print(text);
}

void formatTime(uint32_t totalSeconds, char *out, size_t outSize) {
  uint32_t hours = totalSeconds / 3600;
  uint32_t minutes = (totalSeconds % 3600) / 60;
  uint32_t seconds = totalSeconds % 60;
  snprintf(out, outSize, "%02lu:%02lu:%02lu",
           static_cast<unsigned long>(hours % 24),
           static_cast<unsigned long>(minutes),
           static_cast<unsigned long>(seconds));
}

void formatMinutes(uint32_t totalSeconds, char *out, size_t outSize) {
  uint32_t minutes = totalSeconds / 60;
  uint32_t seconds = totalSeconds % 60;
  snprintf(out, outSize, "%02lu:%02lu",
           static_cast<unsigned long>(minutes),
           static_cast<unsigned long>(seconds));
}
} // namespace

void renderClockPage(uint32_t nowMs) {
  const uint32_t totalSeconds = nowMs / 1000;
  char timeBuf[16];
  formatTime(totalSeconds, timeBuf, sizeof(timeBuf));

  drawCenteredText("CLOCK", 14, 1);
  drawCenteredText(timeBuf, 30, 2);
}

void renderPomodoroPage(const PomodoroState &state) {
  char timeBuf[16];
  formatMinutes(state.remainingSec, timeBuf, sizeof(timeBuf));

  drawCenteredText("POMODORO", 10, 1);
  drawCenteredText(timeBuf, 28, 2);
  drawCenteredText(state.running ? "RUNNING" : "PAUSED", 50, 1);
}

void updatePomodoro(PomodoroState &state, uint32_t nowMs) {
  if (!state.running) {
    state.lastTickMs = nowMs;
    return;
  }

  if (nowMs - state.lastTickMs >= 1000) {
    uint32_t elapsed = (nowMs - state.lastTickMs) / 1000;
    state.lastTickMs += elapsed * 1000;
    if (state.remainingSec > elapsed) {
      state.remainingSec -= elapsed;
    } else {
      state.remainingSec = 0;
      state.running = false;
    }
  }
}

void resetPomodoro(PomodoroState &state, uint32_t durationSec) {
  state.running = false;
  state.remainingSec = durationSec;
  state.lastTickMs = millis();
}
