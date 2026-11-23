/*
  ============================================================
                    WALLE FULL BUILD V4
  ============================================================

  Controller: ESP32 + Bluepad32
  Drive:      Two RC-style ESC signals
  Servos:     PCA9685 servo driver on I2C (SDA 21, SCL 22)

  DRIVE CONTROLS:
    Left stick vertical  = Left track
    Right stick vertical = Right track

  SERVO CONTROLS:
    A = Play WALL-E sound track 0001.mp3
    L2 = Toggle left arm
    R2 = Toggle right arm
    L1 = Look left
    R1 = Look right
    L1 + R1 = Look forward
    X = Toggle left eye; hold to center it
    B = Toggle right eye; hold to center it
    Y = Toggle neck up/down
    D-pad = Servo animations

  SERIAL MOTOR CONTROLS:
    W/S/A/D = Forward/reverse/spin left/spin right
    Q/Z     = Left motor forward/reverse
    E/X     = Right motor forward/reverse
    0/SPACE = Stop
    1-9     = Set speed 10%-90%
    C       = Return drive control to Bluetooth
    H       = Print help
*/

#include <Bluepad32.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <U8g2lib.h>
#include <HardwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// ============================================================
//                      USER CONFIGURATION
// ============================================================

// Set this to 2 for a 2S battery or 3 for a 3S battery.
constexpr uint8_t BATTERY_CELL_COUNT = 2;
static_assert(BATTERY_CELL_COUNT == 2 || BATTERY_CELL_COUNT == 3,
              "BATTERY_CELL_COUNT must be 2 or 3");

// ============================================================
//                     DFPLAYER MINI AUDIO
// ============================================================

#define DF_RX 19
#define DF_TX 18
#define DFPLAYER_VOLUME 25

HardwareSerial dfSerial(1);
DFRobotDFPlayerMini dfPlayer;
bool dfPlayerReady = false;
bool dfSerialStarted = false;

// ============================================================
//                     MOTOR / ESC SETTINGS
// ============================================================

//#define LEFT_ESC_PIN  25
//#define RIGHT_ESC_PIN 26
#define LEFT_ESC_PIN  12
#define RIGHT_ESC_PIN 13

#define ESC_MIN          1000
#define ESC_NEUTRAL      1500
#define ESC_MAX          2000
#define ESC_REFRESH_RATE 50
#define ESC_ARM_TIME_MS  3000

#define DEADZONE 40

// Both physical motor directions are reversed.
#define INVERT_LEFT_MOTOR  true
#define INVERT_RIGHT_MOTOR true

Servo leftESC;
Servo rightESC;

int serialSpeedPercent = 50;
bool serialControlMode = false;
bool escArmed = false;
unsigned long escArmStartTime = 0;

// ============================================================
//                      PCA9685 SERVOS
// ============================================================

#define I2C_SDA 21
#define I2C_SCL 22
#define PCA9685_ADDRESS 0x40
#define SERVO_FREQUENCY 50
#define SERVO_MIN 150
#define SERVO_MAX 600

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(PCA9685_ADDRESS);
bool pca9685Ready = false;

// ============================================================
//                  OLED BATTERY DISPLAY
// ============================================================

#define BATTERY_PIN 14
#define R1 100000.0
#define R2 6800.0
#define BATTERY_CALIBRATION 0.9594
#define BATTERY_ADC_SAMPLES 16
#define NO_VOLTAGE_MV 10
#define NO_VOLTAGE_PERCENT 0
#define BATTERY_DISPLAY_INTERVAL_MS 500
#define BATTERY_SERIAL_INTERVAL_MS 5000

// Preserve the original 3.3 V-per-cell empty and 4.2 V-per-cell full points,
// while scaling them automatically for the selected 2S or 3S battery.
constexpr float BATTERY_EMPTY_VOLTS = 3.3f * BATTERY_CELL_COUNT;
constexpr float BATTERY_FULL_VOLTS = 4.2f * BATTERY_CELL_COUNT;

// This module works best with the SH1106 128x64 mapping. SSD1309 NONAME0 and
// NONAME2 retained the same fixed lines, while SH1107 was substantially worse.
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(
  // Rotate the native 128x64 panel into the 64x128 chest-panel orientation.
  U8G2_R3,
  U8X8_PIN_NONE
);

unsigned long lastBatteryDisplayTime = 0;
unsigned long lastBatterySerialTime = 0;
bool oledReady = false;

// Slow peripherals are brought online from loop() so Bluetooth and the ESC
// neutral pulses keep running while the rest of the hardware powers up.
enum StartupStage {
  START_WIRE,
  CHECK_PCA,
  WAIT_FOR_PCA,
  START_OLED,
  START_DF_SERIAL,
  START_DFPLAYER,
  STARTUP_COMPLETE
};

StartupStage startupStage = START_WIRE;
unsigned long startupStageTime = 0;

void drawCenteredText(const char *text, uint8_t baselineY) {
  int textX = (64 - u8g2.getStrWidth(text)) / 2;
  u8g2.drawStr(textX, baselineY, text);
}

void drawSun() {
  const int centerX = 9;
  const int centerY = 41;

  // Small ring-and-ray sun beside the top of the portrait charge stack.
  u8g2.drawCircle(centerX, centerY, 4);
  u8g2.drawDisc(centerX, centerY, 1);
  u8g2.drawLine(centerX, 34, centerX, 31);
  u8g2.drawLine(centerX, 48, centerX, 51);
  u8g2.drawLine(2, centerY, 0, centerY);
  u8g2.drawLine(16, centerY, 18, centerY);
  u8g2.drawLine(4, 36, 2, 34);
  u8g2.drawLine(14, 36, 16, 34);
  u8g2.drawLine(4, 46, 2, 48);
  u8g2.drawLine(14, 46, 16, 48);
}

void drawChargeBars(int batteryLevel) {
  const uint8_t barCount = 10;
  const uint8_t barX = 21;
  const uint8_t barWidth = 40;
  const uint8_t barHeight = 7;
  const uint8_t barSpacing = 2;
  const uint8_t topY = 32;

  // Each complete 10% lights one bar, filling upward from the bottom.
  int filledBars = constrain(batteryLevel / 10, 0, barCount);

  for (uint8_t bar = 0; bar < barCount; bar++) {
    uint8_t y = topY + bar * (barHeight + barSpacing);
    // Array index 9 is the bottom bar, so higher charge climbs upward. Empty
    // levels remain black; active levels are completely solid rectangles.
    if (bar >= barCount - filledBars) {
      u8g2.drawBox(barX, y, barWidth, barHeight);
    }
  }
}

void displayLevel(int batteryLevel) {
  // This is a full-framebuffer (_F_) U8g2 display, so clear and transmit one
  // complete frame instead of using the paged firstPage()/nextPage() API.
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);
  // This compact font rendered cleanly across the entire rotated display.
  // Two lines preserve readability without crowding the charge scale.
  u8g2.setFont(u8g2_font_5x7_tr);
  drawCenteredText("SOLAR CHARGE", 11);
  drawCenteredText("LEVEL", 20);

  drawSun();
  drawChargeBars(batteryLevel);
  u8g2.sendBuffer();
}

uint32_t readBatteryMilliVolts() {
  uint32_t totalMilliVolts = 0;
  for (uint8_t sample = 0; sample < BATTERY_ADC_SAMPLES; sample++) {
    totalMilliVolts += analogReadMilliVolts(BATTERY_PIN);
  }
  return totalMilliVolts / BATTERY_ADC_SAMPLES;
}

float batteryVoltageFromAdc(uint32_t adcMilliVolts) {
  float adcVoltage = adcMilliVolts / 1000.0;
  return adcVoltage * ((R1 + R2) / R2) * BATTERY_CALIBRATION;
}

int batteryPercentFromVoltage(float batteryVoltage) {
  if (batteryVoltage >= BATTERY_FULL_VOLTS) return 100;
  if (batteryVoltage <= BATTERY_EMPTY_VOLTS) return 0;
  return (int)((batteryVoltage - BATTERY_EMPTY_VOLTS) * 100.0f /
               (BATTERY_FULL_VOLTS - BATTERY_EMPTY_VOLTS));
}

int readBatteryPercent() {
  uint32_t adcMilliVolts = readBatteryMilliVolts();

  // A grounded/unpowered measurement input can contain a few millivolts of
  // noise, so treat anything this close to zero as no measurement voltage.
  if (adcMilliVolts <= NO_VOLTAGE_MV) return NO_VOLTAGE_PERCENT;

  return batteryPercentFromVoltage(batteryVoltageFromAdc(adcMilliVolts));
}

void updateBatteryDisplay() {
  if (!oledReady) return;
  unsigned long now = millis();
  if (now - lastBatteryDisplayTime >= BATTERY_DISPLAY_INTERVAL_MS) {
    lastBatteryDisplayTime = now;
    displayLevel(readBatteryPercent());
  }

  if (now - lastBatterySerialTime >= BATTERY_SERIAL_INTERVAL_MS) {
    lastBatterySerialTime = now;
    uint32_t adcMilliVolts = readBatteryMilliVolts();
    float batteryVoltage = batteryVoltageFromAdc(adcMilliVolts);
    Serial.print("Battery: ADC=");
    Serial.print(adcMilliVolts);
    Serial.print(" mV (");
    Serial.print(adcMilliVolts / 1000.0, 3);
    Serial.print(" V, calculated=");
    Serial.print(batteryVoltage, 2);
    Serial.print(" V, level=");
    Serial.print(adcMilliVolts <= NO_VOLTAGE_MV
                   ? NO_VOLTAGE_PERCENT
                   : batteryPercentFromVoltage(batteryVoltage));
    Serial.println("%");
  }
}

// PCA9685 channels from WallefullbuildV2.
#define HEAD_ROTATE_CH  0
#define UPPER_NECK_CH   1
#define LOWER_NECK_CH   2
#define RIGHT_EYE_CH    3
#define LEFT_EYE_CH     4
#define LEFT_ARM_CH     5
#define RIGHT_ARM_CH    6

// Calibrated positions from WallefullbuildV2.
#define HEAD_LEFT          95
#define HEAD_FORWARD       65
#define HEAD_RIGHT         35
#define NECK_TOP_EXTEND    15
#define NECK_TOP_LOWER     70
#define NECK_BOTTOM_EXTEND 150
#define NECK_BOTTOM_LOWER  40
#define RIGHT_EYE_RAISED   70
#define RIGHT_EYE_LOWERED  30
#define LEFT_EYE_RAISED    60
#define LEFT_EYE_LOWERED   30
#define LEFT_ARM_RAISED    20
#define LEFT_ARM_LOWERED   90
#define RIGHT_ARM_RAISED   130
#define RIGHT_ARM_LOWERED  60

const unsigned long SERVO_TIMEOUT_MS = 1500;
const unsigned long EYE_HOLD_MS = 500;

unsigned long lastServoTime[16] = {0};
bool servoEnabled[16] = {false};

bool leftArmUp = false;
bool rightArmUp = false;
bool leftEyeUp = false;
bool rightEyeUp = false;
bool neckUp = false;

// Previous button states provide one action per press.
bool previousL2 = false;
bool previousR2 = false;
bool previousY = false;
bool previousX = false;
bool previousB = false;
bool previousA = false;
int previousDpad = 0;
unsigned long xPressTime = 0;
unsigned long bPressTime = 0;
bool xHoldHandled = false;
bool bHoldHandled = false;

// D-pad animation state.
bool animA = false;
bool animB = false;
bool animX = false;
bool animY = false;
unsigned long animTimers[4] = {0};
int animSteps[4] = {-1, -1, -1, -1};

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

// ============================================================
//                     MOTOR FUNCTIONS
// ============================================================

int invertESCSignal(int signal) {
  return (2 * ESC_NEUTRAL) - signal;
}

void setMotors(int leftSignal, int rightSignal) {
  leftSignal = constrain(leftSignal, ESC_MIN, ESC_MAX);
  rightSignal = constrain(rightSignal, ESC_MIN, ESC_MAX);

  if (INVERT_LEFT_MOTOR) leftSignal = invertESCSignal(leftSignal);
  if (INVERT_RIGHT_MOTOR) rightSignal = invertESCSignal(rightSignal);

  leftESC.writeMicroseconds(leftSignal);
  rightESC.writeMicroseconds(rightSignal);
}

void stopMotors() {
  setMotors(ESC_NEUTRAL, ESC_NEUTRAL);
}

void attachEscOutputs() {
  leftESC.setPeriodHertz(ESC_REFRESH_RATE);
  rightESC.setPeriodHertz(ESC_REFRESH_RATE);
  leftESC.attach(LEFT_ESC_PIN, ESC_MIN, ESC_MAX);
  rightESC.attach(RIGHT_ESC_PIN, ESC_MIN, ESC_MAX);
  stopMotors();
}

void updateEscArming() {
  if (escArmed) return;

  // Keep both ESCs neutral while Bluepad32 and the other hardware continue
  // running normally during the arming period.
  stopMotors();

  if (millis() - escArmStartTime >= ESC_ARM_TIME_MS) {
    escArmed = true;
    Serial.println("ESC arming complete. Motor control enabled.");
  }
}

int serialForwardSignal() {
  return map(serialSpeedPercent, 0, 100, ESC_NEUTRAL, ESC_MAX);
}

int serialReverseSignal() {
  return map(serialSpeedPercent, 0, 100, ESC_NEUTRAL, ESC_MIN);
}

int applyDeadzone(int value) {
  return abs(value) <= DEADZONE ? 0 : value;
}

int throttleToESC(int throttle) {
  throttle = constrain(throttle, -512, 512);
  if (throttle == 0) return ESC_NEUTRAL;
  if (throttle > 0) {
    return map(throttle, DEADZONE, 512, ESC_NEUTRAL, ESC_MAX);
  }
  return map(throttle, -512, -DEADZONE, ESC_MIN, ESC_NEUTRAL);
}

// ============================================================
//                      SERVO FUNCTIONS
// ============================================================

uint16_t angleToPulse(int angle) {
  angle = constrain(angle, 0, 180);
  return map(angle, 0, 180, SERVO_MIN, SERVO_MAX);
}

bool isServoActive(uint8_t channel) {
  return servoEnabled[channel] &&
         (millis() - lastServoTime[channel] <= SERVO_TIMEOUT_MS);
}

int getActiveServoCount() {
  int count = 0;
  for (uint8_t channel = 0; channel < 16; channel++) {
    if (isServoActive(channel)) count++;
  }
  return count;
}

bool canMoveServo(uint8_t channel) {
  if (channel >= 16) return false;
  if (isServoActive(channel)) return true;
  return getActiveServoCount() < 2;
}

bool moveServo(uint8_t channel, int angle) {
  if (!pca9685Ready) return false;
  if (!canMoveServo(channel)) return false;
  pwm.setPWM(channel, 0, angleToPulse(angle));
  lastServoTime[channel] = millis();
  servoEnabled[channel] = true;
  return true;
}

bool moveTwoServos(uint8_t channel1, int angle1,
                   uint8_t channel2, int angle2) {
  // Start the pair only when both available slots are free (or already active).
  int neededSlots = 0;
  if (!isServoActive(channel1)) neededSlots++;
  if (channel2 != channel1 && !isServoActive(channel2)) neededSlots++;
  if (getActiveServoCount() + neededSlots > 2) return false;

  bool firstMoved = moveServo(channel1, angle1);
  bool secondMoved = moveServo(channel2, angle2);
  return firstMoved && secondMoved;
}

void stopInactiveServos() {
  if (!pca9685Ready) return;

  unsigned long now = millis();
  for (uint8_t channel = 0; channel < 16; channel++) {
    if (servoEnabled[channel] &&
        now - lastServoTime[channel] > SERVO_TIMEOUT_MS) {
      pwm.setPWM(channel, 0, 0);
      servoEnabled[channel] = false;
    }
  }
}

// ============================================================
//                    SERVO ANIMATIONS
// ============================================================

void handleAnimationA() {
  if (!animA) return;
  unsigned long now = millis();

  switch (animSteps[0]) {
    case 0:
      if (moveTwoServos(UPPER_NECK_CH, NECK_TOP_EXTEND,
                        LOWER_NECK_CH, NECK_BOTTOM_EXTEND)) {
        animTimers[0] = now;
        animSteps[0]++;
      }
      break;
    case 1:
      if (now - animTimers[0] >= 200 &&
          moveTwoServos(LEFT_ARM_CH, LEFT_ARM_RAISED,
                        RIGHT_ARM_CH, RIGHT_ARM_RAISED)) {
        animTimers[0] = now;
        animSteps[0]++;
      }
      break;
    case 2:
      if (now - animTimers[0] >= 500 && moveServo(HEAD_ROTATE_CH, HEAD_LEFT)) {
        animTimers[0] = now;
        animSteps[0]++;
      }
      break;
    case 3:
      if (now - animTimers[0] >= 500 &&
          moveTwoServos(RIGHT_EYE_CH, RIGHT_EYE_RAISED,
                        LEFT_EYE_CH, LEFT_EYE_RAISED)) {
        animTimers[0] = now;
        animSteps[0]++;
      }
      break;
    case 4:
      if (now - animTimers[0] >= 1000 &&
          moveTwoServos(RIGHT_EYE_CH, RIGHT_EYE_LOWERED,
                        LEFT_EYE_CH, LEFT_EYE_LOWERED)) {
        animTimers[0] = now;
        animSteps[0]++;
      }
      break;
    case 5:
      if (now - animTimers[0] >= 400 && moveServo(HEAD_ROTATE_CH, HEAD_FORWARD)) {
        animTimers[0] = now;
        animSteps[0]++;
      }
      break;
    case 6:
      if (now - animTimers[0] >= 200 &&
          moveTwoServos(UPPER_NECK_CH, NECK_TOP_LOWER,
                        LOWER_NECK_CH, NECK_BOTTOM_LOWER)) {
        animTimers[0] = now;
        animSteps[0]++;
      }
      break;
    case 7:
      if (now - animTimers[0] >= 400 &&
          moveTwoServos(LEFT_ARM_CH, LEFT_ARM_LOWERED,
                        RIGHT_ARM_CH, RIGHT_ARM_LOWERED)) {
        animA = false;
        animSteps[0] = -1;
      }
      break;
  }
}

void handleAnimationB() {
  if (!animB) return;
  unsigned long now = millis();

  if (animSteps[1] == 0 && moveServo(HEAD_ROTATE_CH, HEAD_RIGHT)) {
    animTimers[1] = now;
    animSteps[1]++;
  } else if (animSteps[1] == 1 && now - animTimers[1] >= 400 &&
             moveServo(HEAD_ROTATE_CH, HEAD_FORWARD)) {
    animTimers[1] = now;
    animSteps[1]++;
  } else if (animSteps[1] == 2 && now - animTimers[1] >= 400 &&
             moveTwoServos(RIGHT_EYE_CH, RIGHT_EYE_RAISED,
                           LEFT_EYE_CH, LEFT_EYE_RAISED)) {
    animTimers[1] = now;
    animSteps[1]++;
  } else if (animSteps[1] == 3 && now - animTimers[1] >= 400 &&
             moveTwoServos(RIGHT_EYE_CH, RIGHT_EYE_LOWERED,
                           LEFT_EYE_CH, LEFT_EYE_LOWERED)) {
    animB = false;
    animSteps[1] = -1;
  }
}

void handleAnimationX() {
  if (!animX) return;
  unsigned long now = millis();

  if (animSteps[2] == 0 &&
      moveTwoServos(LEFT_ARM_CH, LEFT_ARM_RAISED,
                    RIGHT_ARM_CH, RIGHT_ARM_RAISED)) {
    animTimers[2] = now;
    animSteps[2]++;
  } else if (animSteps[2] == 1 && now - animTimers[2] >= 400 &&
             moveTwoServos(LEFT_ARM_CH, LEFT_ARM_LOWERED,
                           RIGHT_ARM_CH, RIGHT_ARM_LOWERED)) {
    animX = false;
    animSteps[2] = -1;
  }
}

void handleAnimationY() {
  if (!animY) return;
  unsigned long now = millis();

  if (animSteps[3] == 0 && moveServo(HEAD_ROTATE_CH, HEAD_LEFT)) {
    animTimers[3] = now;
    animSteps[3]++;
  } else if (animSteps[3] == 1 && now - animTimers[3] >= 400 &&
             moveServo(HEAD_ROTATE_CH, HEAD_RIGHT)) {
    animTimers[3] = now;
    animSteps[3]++;
  } else if (animSteps[3] == 2 && now - animTimers[3] >= 400 &&
             moveServo(HEAD_ROTATE_CH, HEAD_FORWARD)) {
    animY = false;
    animSteps[3] = -1;
  }
}

void handleAnimations() {
  handleAnimationA();
  handleAnimationB();
  handleAnimationX();
  handleAnimationY();
}

// ============================================================
//                  BLUETOOTH CONTROLLER
// ============================================================

void resetButtonState() {
  previousL2 = false;
  previousR2 = false;
  previousY = false;
  previousX = false;
  previousB = false;
  previousA = false;
  previousDpad = 0;
  xPressTime = 0;
  bPressTime = 0;
  xHoldHandled = false;
  bHoldHandled = false;
}

void onConnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == nullptr) {
      myControllers[i] = ctl;
      stopMotors();
      resetButtonState();
      Serial.println("Bluetooth controller connected.");
      break;
    }
  }
}

void onDisconnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == ctl) {
      myControllers[i] = nullptr;
      break;
    }
  }
  stopMotors();
  resetButtonState();
  Serial.println("Bluetooth controller disconnected. Motors stopped.");
}

void processServoControls(ControllerPtr ctl) {
  bool aPressed = ctl->a();
  bool l2Pressed = ctl->l2() > 0;
  bool r2Pressed = ctl->r2() > 0;
  bool yPressed = ctl->y();
  bool xPressed = ctl->x();
  bool bPressed = ctl->b();

  // Play once on the rising edge so holding A does not repeatedly restart
  // the first track on the DFPlayer's microSD card.
  if (aPressed && !previousA) {
    if (dfPlayerReady) {
      dfPlayer.play(1);
      Serial.println("AUDIO | Playing track 0001.mp3");
    } else {
      Serial.println("AUDIO | DFPlayer is not available");
    }
  }

  if (l2Pressed && !previousL2) {
    leftArmUp = !leftArmUp;
    moveServo(LEFT_ARM_CH, leftArmUp ? LEFT_ARM_RAISED : LEFT_ARM_LOWERED);
  }

  if (r2Pressed && !previousR2) {
    rightArmUp = !rightArmUp;
    moveServo(RIGHT_ARM_CH, rightArmUp ? RIGHT_ARM_RAISED : RIGHT_ARM_LOWERED);
  }

  if (ctl->l1() && ctl->r1()) {
    moveServo(HEAD_ROTATE_CH, HEAD_FORWARD);
  } else if (ctl->l1()) {
    moveServo(HEAD_ROTATE_CH, HEAD_LEFT);
  } else if (ctl->r1()) {
    moveServo(HEAD_ROTATE_CH, HEAD_RIGHT);
  }

  if (xPressed && !previousX) {
    xPressTime = millis();
    xHoldHandled = false;
    leftEyeUp = !leftEyeUp;
    moveServo(LEFT_EYE_CH, leftEyeUp ? LEFT_EYE_RAISED : LEFT_EYE_LOWERED);
  } else if (xPressed && !xHoldHandled && millis() - xPressTime > EYE_HOLD_MS) {
    moveServo(LEFT_EYE_CH, (LEFT_EYE_RAISED + LEFT_EYE_LOWERED) / 2);
    xHoldHandled = true;
  } else if (!xPressed) {
    xPressTime = 0;
    xHoldHandled = false;
  }

  if (bPressed && !previousB) {
    bPressTime = millis();
    bHoldHandled = false;
    rightEyeUp = !rightEyeUp;
    moveServo(RIGHT_EYE_CH, rightEyeUp ? RIGHT_EYE_RAISED : RIGHT_EYE_LOWERED);
  } else if (bPressed && !bHoldHandled && millis() - bPressTime > EYE_HOLD_MS) {
    moveServo(RIGHT_EYE_CH, (RIGHT_EYE_RAISED + RIGHT_EYE_LOWERED) / 2);
    bHoldHandled = true;
  } else if (!bPressed) {
    bPressTime = 0;
    bHoldHandled = false;
  }

  if (yPressed && !previousY) {
    neckUp = !neckUp;
    moveTwoServos(UPPER_NECK_CH, neckUp ? NECK_TOP_EXTEND : NECK_TOP_LOWER,
                  LOWER_NECK_CH, neckUp ? NECK_BOTTOM_EXTEND : NECK_BOTTOM_LOWER);
  }

  int dpad = ctl->dpad();
  if (dpad != previousDpad) {
    if (dpad == DPAD_UP && !animA) {
      animA = true;
      animSteps[0] = 0;
    } else if (dpad == DPAD_RIGHT && !animB) {
      animB = true;
      animSteps[1] = 0;
    } else if (dpad == DPAD_DOWN && !animX) {
      animX = true;
      animSteps[2] = 0;
    } else if (dpad == DPAD_LEFT && !animY) {
      animY = true;
      animSteps[3] = 0;
    }
  }

  previousL2 = l2Pressed;
  previousR2 = r2Pressed;
  previousY = yPressed;
  previousX = xPressed;
  previousB = bPressed;
  previousA = aPressed;
  previousDpad = dpad;
}

void processControllers() {
  for (auto ctl : myControllers) {
    if (!(ctl && ctl->isConnected()) || !ctl->hasData()) continue;

    // Servo controls remain available even while serial controls the motors.
    processServoControls(ctl);

    if (!escArmed) {
      stopMotors();
    } else if (!serialControlMode) {
      int leftThrottle = applyDeadzone(-ctl->axisY());
      int rightThrottle = applyDeadzone(-ctl->axisRY());
      setMotors(throttleToESC(leftThrottle), throttleToESC(rightThrottle));
    }
  }
}

// ============================================================
//                       SERIAL CONTROL
// ============================================================

void printSerialHelp() {
  Serial.println();
  Serial.println("================================================");
  Serial.println("              WALLE FULL BUILD V4");
  Serial.println("================================================");
  Serial.println("W/S/A/D = Forward/reverse/spin left/spin right");
  Serial.println("Q/Z = Left motor forward/reverse");
  Serial.println("E/X = Right motor forward/reverse");
  Serial.println("0 or SPACE = Stop motors");
  Serial.println("1-9 = Set serial speed 10%-90%");
  Serial.println("C = Bluetooth drive mode");
  Serial.println("H = Print help");
  Serial.print("Current serial speed: ");
  Serial.print(serialSpeedPercent);
  Serial.println("%");
  Serial.println("================================================");
}

void processSerial() {
  while (Serial.available()) {
    char command = Serial.read();
    if (command >= 'A' && command <= 'Z') command += ('a' - 'A');
    if (command == '\n' || command == '\r') continue;

    if (command >= '1' && command <= '9') {
      serialSpeedPercent = (command - '0') * 10;
      Serial.print("Serial speed set to ");
      Serial.print(serialSpeedPercent);
      Serial.println("%");
      continue;
    }

    if (command == 'c') {
      stopMotors();
      serialControlMode = false;
      Serial.println("CONTROL MODE: BLUETOOTH. Motors stopped.");
      continue;
    }

    if (command == 'h') {
      printSerialHelp();
      continue;
    }

    bool isMotorCommand =
      command == 'w' || command == 's' ||
      command == 'a' || command == 'd' ||
      command == 'q' || command == 'z' ||
      command == 'e' || command == 'x' ||
      command == '0' || command == ' ';

    if (isMotorCommand && !escArmed) {
      stopMotors();
      Serial.println("ESCs are still arming. Motor command ignored.");
      continue;
    }

    if (isMotorCommand) {
      serialControlMode = true;
    }
    int forward = serialForwardSignal();
    int reverse = serialReverseSignal();

    if (command == 'w') {
      setMotors(forward, forward);
      Serial.println("SERIAL | FORWARD");
    } else if (command == 's') {
      setMotors(reverse, reverse);
      Serial.println("SERIAL | REVERSE");
    } else if (command == 'a') {
      setMotors(reverse, forward);
      Serial.println("SERIAL | SPIN LEFT");
    } else if (command == 'd') {
      setMotors(forward, reverse);
      Serial.println("SERIAL | SPIN RIGHT");
    } else if (command == 'q') {
      setMotors(forward, ESC_NEUTRAL);
      Serial.println("SERIAL | LEFT MOTOR FORWARD");
    } else if (command == 'z') {
      setMotors(reverse, ESC_NEUTRAL);
      Serial.println("SERIAL | LEFT MOTOR REVERSE");
    } else if (command == 'e') {
      setMotors(ESC_NEUTRAL, forward);
      Serial.println("SERIAL | RIGHT MOTOR FORWARD");
    } else if (command == 'x') {
      setMotors(ESC_NEUTRAL, reverse);
      Serial.println("SERIAL | RIGHT MOTOR REVERSE");
    } else if (command == '0' || command == ' ') {
      stopMotors();
      Serial.println("SERIAL | MOTORS STOPPED");
    } else {
      Serial.print("Unknown command: ");
      Serial.println(command);
      Serial.println("Enter H for help.");
    }
  }
}

// ============================================================
//                         SETUP / LOOP
// ============================================================

void updateDeferredHardware() {
  unsigned long now = millis();

  switch (startupStage) {
    case START_WIRE:
      // Give the battery-powered 3.3 V and 5 V rails time to settle while
      // controller servicing and ESC neutral output are already active.
      if (now - startupStageTime < 250) return;
      Wire.begin(I2C_SDA, I2C_SCL);
      // Use standard-mode I2C on the shared OLED/PCA9685 wiring. The lower
      // rate provides substantially more signal margin than the 400 kHz
      // default on the longer finished-build bus.
      Wire.setClock(100000);
      startupStageTime = now;
      startupStage = CHECK_PCA;
      Serial.println("I2C bus started.");
      break;

    case CHECK_PCA:
      // Match the known-working ServoTester settling time without blocking.
      if (now - startupStageTime < 200) return;
      Wire.beginTransmission(PCA9685_ADDRESS);
      if (Wire.endTransmission() != 0) {
        Serial.println("ERROR: PCA9685 not detected at 0x40.");
        Serial.println("Servo controls disabled; startup will continue.");
        startupStage = START_OLED;
      } else {
        pwm.begin();
        pwm.setPWMFreq(SERVO_FREQUENCY);
        startupStageTime = now;
        startupStage = WAIT_FOR_PCA;
      }
      break;

    case WAIT_FOR_PCA:
      if (now - startupStageTime < 500) return;
      pca9685Ready = true;
      Serial.println("PCA9685 detected and initialized successfully.");
      startupStage = START_OLED;
      break;

    case START_OLED:
      analogReadResolution(12);
      // Battery-only calibration on this ESP32 showed that 6 dB produces
      // accurate GPIO14 readings; 0 dB saturated near 1.24 V.
      analogSetPinAttenuation(BATTERY_PIN, ADC_6db);
      u8g2.setBusClock(100000);
      u8g2.begin();
      oledReady = true;
      displayLevel(readBatteryPercent());
      lastBatteryDisplayTime = now;
      Serial.println("OLED battery display initialized.");
      startupStageTime = now;
      startupStage = START_DF_SERIAL;
      break;

    case START_DF_SERIAL:
      // Audio starts last so it cannot interfere with controller reconnect or
      // the ESC arming signal during the supply's slow power-up.
      if (now - startupStageTime < 500) return;
      dfSerial.begin(9600, SERIAL_8N1, DF_RX, DF_TX);
      dfPlayer.setTimeOut(250);
      dfSerialStarted = true;
      startupStageTime = now;
      startupStage = START_DFPLAYER;
      Serial.println("DFPlayer serial port started.");
      break;

    case START_DFPLAYER:
      if (now - startupStageTime < 500) return;
      // No reset and no ACK verification: this sends commands without waiting
      // for a response from modules that omit or delay their status reply.
      dfPlayer.begin(dfSerial, false, false);
      dfPlayer.volume(DFPLAYER_VOLUME);
      dfPlayerReady = true;
      startupStage = STARTUP_COMPLETE;
      Serial.println("DFPlayer command mode ready. A button plays 0001.mp3.");
      Serial.println("All deferred hardware startup stages complete.");
      break;

    case STARTUP_COMPLETE:
      break;
  }
}

void setup() {
  // Reproduce the known-working MotorDriverMotorTest startup exactly: attach
  // both ESCs immediately, send neutral, and leave that PWM uninterrupted for
  // the full arming interval before Bluetooth or peripheral initialization.
  attachEscOutputs();
  escArmStartTime = millis();

  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting WALLE Full Build V4...");
  Serial.println("ESC outputs initialized. Sending uninterrupted neutral...");
  delay(ESC_ARM_TIME_MS);
  stopMotors();
  escArmed = true;
  Serial.println("ESC arming hold complete. Motor control enabled.");

  // Start Bluetooth immediately instead of waiting for ESC arming and the
  // I2C devices. Preserve pairing keys so the controller reconnects after
  // the ESP32 reset button is pressed or power is cycled.
  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.enableVirtualDevice(false);
  Serial.println("Bluepad32 initialized. Stored controller pairing preserved.");

  // Use this only for a deliberate one-time pairing reset, then comment it
  // again before subsequent uploads:
  // BP32.forgetBluetoothKeys();

  startupStageTime = millis();
  Serial.println("Controller service active; starting peripherals in stages.");
  printSerialHelp();
}

void loop() {
  BP32.update();
  updateEscArming();
  updateDeferredHardware();
  // Disable timed-out PWM channels before admitting any new servo movement.
  stopInactiveServos();
  processSerial();
  processControllers();
  handleAnimations();
  updateBatteryDisplay();
  delay(5);
}
