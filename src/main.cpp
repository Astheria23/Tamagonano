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
static constexpr int kPageBuzzerChannel = 4;
static constexpr int kPageBuzzerResolution = 8;
static constexpr int kPageBuzzerFreq = 880;
static constexpr uint32_t kTouchDebounceMs = 40;
static constexpr uint16_t kLdrMinContrast = 16;
static constexpr uint16_t kLdrMaxContrast = 255;
static constexpr uint32_t kLdrLogIntervalMs = 1000;
static constexpr uint32_t kPettingHoldMs = 1200;
static constexpr uint32_t kInputIgnoreMs = 600;
static constexpr float kServoCenterDeg = 90.0f;
static constexpr float kServoMaxDeltaDeg = 15.0f;
static constexpr float kServoGain = 0.8f;

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

static bool readPressed(uint8_t pin, bool &lastStable, uint32_t &lastChangeMs) {
  const bool rawPressed = digitalRead(pin) == LOW;
  const uint32_t nowMs = millis();
  if (rawPressed != lastStable && nowMs - lastChangeMs >= kTouchDebounceMs) {
    lastStable = rawPressed;
    lastChangeMs = nowMs;
    return rawPressed;
  }
  return false;
}

static void handleTouchInput(uint32_t nowMs, uint32_t bootMs, uint32_t &pettingUntilMs) {
  static bool t1Stable = false;
  static bool t2Stable = false;
  static bool t3Stable = false;
  static uint32_t t1ChangeMs = 0;
  static uint32_t t2ChangeMs = 0;
  static uint32_t t3ChangeMs = 0;
  static bool initialized = false;

  if (nowMs - bootMs < kInputIgnoreMs) {
    return;
  }

  if (!initialized) {
    t1Stable = digitalRead(TOUCH1_PIN) == LOW;
    t2Stable = digitalRead(TOUCH2_PIN) == LOW;
    t3Stable = digitalRead(TOUCH3_PIN) == LOW;
    t1ChangeMs = nowMs;
    t2ChangeMs = nowMs;
    t3ChangeMs = nowMs;
    initialized = true;
    return;
  }

  if (readPressed(TOUCH1_PIN, t1Stable, t1ChangeMs)) {
    currentPage = static_cast<PageType>((currentPage + 2) % 3);
    playPageBeep();
  }

  if (readPressed(TOUCH2_PIN, t2Stable, t2ChangeMs)) {
    currentPage = static_cast<PageType>((currentPage + 1) % 3);
    playPageBeep();
  }

  if (readPressed(TOUCH3_PIN, t3Stable, t3ChangeMs)) {
    if (currentPage == PAGE_POMODORO) {
      if (pomodoroState.remainingSec == 0) {
        resetPomodoro(pomodoroState, kPomodoroDurationSec);
        pomodoroState.running = true;
        pomodoroState.lastTickMs = millis();
      } else {
        pomodoroState.running = !pomodoroState.running;
        pomodoroState.lastTickMs = millis();
      }
    } else if (currentPage == PAGE_FACE) {
      if (nowMs > pettingUntilMs) {
        pettingUntilMs = nowMs + kPettingHoldMs;
        playHum();
      }
    }
  }
}

static int updateLdr(uint32_t nowMs) {
  static uint32_t lastLogMs = 0;
  const int raw = analogRead(LDR_PIN);
  const uint16_t contrast = static_cast<uint16_t>(map(raw, 0, 4095, kLdrMinContrast, kLdrMaxContrast));
  display.ssd1306_command(SSD1306_SETCONTRAST);
  display.ssd1306_command(contrast);

  if (nowMs - lastLogMs >= kLdrLogIntervalMs) {
    lastLogMs = nowMs;
    Serial.print("LDR raw: ");
    Serial.print(raw);
    Serial.print(" contrast: ");
    Serial.println(contrast);
  }

  return raw;
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setRotation(2); 

  showSplash();

  wifiConnected = false;
  ntpSynced = false;

  pinMode(TOUCH1_PIN, INPUT_PULLUP);
  pinMode(TOUCH2_PIN, INPUT_PULLUP);
  pinMode(TOUCH3_PIN, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT);

  ESP32PWM::allocateTimer(1);
  neck.setPeriodHertz(50);
  neck.attach(SERVO_PIN, 544, 2400);
  
  neck.write(90);

  resetPomodoro(pomodoroState, kPomodoroDurationSec);
}

void loop() {
  static uint32_t pettingUntilMs = 0;
  static const uint32_t bootMs = millis();
  static float lastFaceServo = 90.0f;
  static float lastFaceX = 0.0f;
  static float lastFaceY = 0.0f;
  static float lastFaceW = 28.0f;
  static float lastFaceH = 28.0f;
  static float lastFaceBlink = 0.0f;
  static PageType lastPage = PAGE_FACE;
  static bool returningToFace = false;
  const uint32_t nowMs = millis();

  handleSerialInput();
  handleTouchInput(nowMs, bootMs, pettingUntilMs);
  updateLdr(nowMs);

  updatePomodoro(pomodoroState, nowMs);

  if (currentPage == PAGE_FACE) {
    if (lastPage != PAGE_FACE) {
      curX = 0;
      curY = 0;
      curW = 28;
      curH = 28;
      curBlink = 0.0f;
      tarX = 0;
      tarY = 0;
      tarW = 28;
      tarH = 28;
      tarBlink = 0.0f;
      curS = kServoCenterDeg;
      tarS = kServoCenterDeg;
      lastFaceX = 0.0f;
      lastFaceY = 0.0f;
      lastFaceW = 28.0f;
      lastFaceH = 28.0f;
      lastFaceBlink = 0.0f;
      lastFaceServo = kServoCenterDeg;
      returningToFace = true;
    }
    if (returningToFace) {
      returningToFace = false;
    } else {
      handleIdleLogic();
    }
    if (nowMs < pettingUntilMs) {
      const int wobble = (nowMs / 120) % 2 == 0 ? -2 : 2;
      tarX = wobble;
      tarY = wobble / 2;
      tarW = 34;
      tarH = 18;
      tarBlink = 0.0f;
    } else {
      tarW = 28;
      tarH = 28;
    }
    tarS = constrain(kServoCenterDeg - (tarX * kServoGain),
             kServoCenterDeg - kServoMaxDeltaDeg,
             kServoCenterDeg + kServoMaxDeltaDeg);
  } else {
    tarX = 0;
    tarY = 0;
    tarW = 28;
    tarH = 28;
    tarS = kServoCenterDeg;
  }

  lastPage = currentPage;

  updatePhysics(neck); 

  if (currentPage == PAGE_FACE) {
    lastFaceX = curX;
    lastFaceY = curY;
    lastFaceW = curW;
    lastFaceH = curH;
    lastFaceBlink = curBlink;
    lastFaceServo = curS;
  }

  display.clearDisplay();
  drawNavbar(85, wifiConnected); 

  if (currentPage == PAGE_FACE) {
    renderFace();
  } else if (currentPage == PAGE_CLOCK) {
    if (ntpSynced) {
      renderClockPageNTP();
    } else {
      renderClockPage(nowMs);
    }
  } else if (currentPage == PAGE_POMODORO) {
    renderPomodoroPage(pomodoroState);
  }

  display.display();
  delay(10);
}