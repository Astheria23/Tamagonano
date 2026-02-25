#ifndef PAGES_H
#define PAGES_H

#include <Arduino.h>
#include "Config.h"

enum PageType : uint8_t {
  PAGE_FACE = 0,
  PAGE_CLOCK = 1,
  PAGE_POMODORO = 2
};

struct PomodoroState {
  bool running;
  uint32_t remainingSec;
  uint32_t lastTickMs;
};

void renderClockPage(uint32_t nowMs);
void renderClockPageNTP();
void renderPomodoroPage(const PomodoroState &state);
void updatePomodoro(PomodoroState &state, uint32_t nowMs);
void resetPomodoro(PomodoroState &state, uint32_t durationSec);

#endif
