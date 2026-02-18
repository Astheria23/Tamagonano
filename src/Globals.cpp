#include "Config.h"

// Global animation state
float curX = 0;
float curY = 0;
float curW = 28;
float curH = 28;
float curS = 90;

float tarX = 0;
float tarY = 0;
float tarW = 28;
float tarH = 28;
float tarS = 90;
float tarBlink = 0;

int lastWrittenServo = -1;
unsigned long lastServoUpdate = 0;

unsigned long nextIdleAction = 0;
unsigned long nextBlinkTime = 0;
unsigned long blinkReleaseTime = 0;

float curBlink = 0;
