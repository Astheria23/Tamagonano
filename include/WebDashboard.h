#ifndef WEB_DASHBOARD_H
#define WEB_DASHBOARD_H

#include <Arduino.h>
#include "Config.h"
#include "Pages.h"

void setupWebServer();
void setWebPageRef(PageType *page);
void setWebPomodoroRef(PomodoroState *state);

#endif
