#ifndef CONFIG_H
#define CONFIG_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

#define SDA_PIN 8
#define SCL_PIN 9
#define SERVO_PIN 4
#define BUZZER_PIN 5
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

extern Adafruit_SSD1306 display;

// Deklarasi extern supaya semua file 'kenal'
extern float curX, curY, curW, curH, curS;
extern float tarX, tarY, tarW, tarH, tarS;
extern int lastWrittenServo;
extern unsigned long lastServoUpdate;

#endif