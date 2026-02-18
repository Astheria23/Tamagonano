#include "Config.h"
#include "Animation.h"

// Global animation state
float curX = 0;
float curY = 0;
float curW = 24;
float curH = 24;
float curS = 90;

float tarX = 0;
float tarY = 0;
float tarW = 24;
float tarH = 24;
float tarS = 90;

int lastWrittenServo = -1;
unsigned long lastServoUpdate = 0;

unsigned long nextIdleAction = 0;
