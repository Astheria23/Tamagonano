#include <Arduino.h>
#include "Config.h"
#include "Face.h"
#include "Animation.h"
#include "Splash.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Servo neck;

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setRotation(2); 

  showSplash();

  ESP32PWM::allocateTimer(1);
  neck.setPeriodHertz(50);
  neck.attach(SERVO_PIN, 544, 2400);
  
  neck.write(90);
}

void loop() {
  handleIdleLogic();

  updatePhysics(neck); 
  
  display.clearDisplay();
  drawNavbar(85, true); 
  renderFace();
  display.display();
  
  delay(10);
}