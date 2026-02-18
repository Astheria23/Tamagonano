#include <Arduino.h>
#include "Config.h"
#include "Face.h"
#include "Animation.h"
#include "Splash.h"
#include "Pages.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Servo neck;

static PageType currentPage = PAGE_FACE;
static PomodoroState pomodoroState = {false, 25 * 60, 0};
static constexpr uint32_t kPomodoroDurationSec = 25 * 60;
static constexpr int kPageBuzzerChannel = 2;
static constexpr int kPageBuzzerResolution = 8;
static constexpr int kPageBuzzerFreq = 880;

static void playPageBeep() {
  static bool buzzerInit = false;
  if (!buzzerInit) {
    ledcSetup(kPageBuzzerChannel, kPageBuzzerFreq, kPageBuzzerResolution);
    ledcAttachPin(BUZZER_PIN, kPageBuzzerChannel);
    buzzerInit = true;
  }
  ledcWriteTone(kPageBuzzerChannel, kPageBuzzerFreq);
  delay(60);
  ledcWriteTone(kPageBuzzerChannel, 0);
}

static void handleSerialInput() {
  while (Serial.available() > 0) {
    char cmd = static_cast<char>(Serial.read());
    if (cmd == '\n' || cmd == '\r') {
      continue;
    }
    if (cmd == '1') {
      currentPage = static_cast<PageType>((currentPage + 2) % 3);
      playPageBeep();
    } else if (cmd == '2') {
      currentPage = static_cast<PageType>((currentPage + 1) % 3);
      playPageBeep();
    } else if (cmd == '3') {
      if (currentPage == PAGE_POMODORO) {
        if (pomodoroState.remainingSec == 0) {
          resetPomodoro(pomodoroState, kPomodoroDurationSec);
          pomodoroState.running = true;
          pomodoroState.lastTickMs = millis();
        } else {
          pomodoroState.running = !pomodoroState.running;
          pomodoroState.lastTickMs = millis();
        }
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setRotation(2); 

  showSplash();

  ESP32PWM::allocateTimer(1);
  neck.setPeriodHertz(50);
  neck.attach(SERVO_PIN, 544, 2400);
  
  neck.write(90);

  resetPomodoro(pomodoroState, kPomodoroDurationSec);
}

void loop() {
  handleSerialInput();

  const uint32_t nowMs = millis();
  updatePomodoro(pomodoroState, nowMs);

  if (currentPage == PAGE_FACE) {
    handleIdleLogic();
  } else {
    tarX = 0;
    tarY = 0;
    tarS = 90;
  }

  updatePhysics(neck); 

  display.clearDisplay();
  drawNavbar(85, true); 

  if (currentPage == PAGE_FACE) {
    renderFace();
  } else if (currentPage == PAGE_CLOCK) {
    renderClockPage(nowMs);
  } else if (currentPage == PAGE_POMODORO) {
    renderPomodoroPage(pomodoroState);
  }

  display.display();
  delay(10);
}