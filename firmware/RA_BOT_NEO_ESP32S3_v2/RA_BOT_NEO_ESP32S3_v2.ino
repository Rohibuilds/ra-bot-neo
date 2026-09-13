/*
  ======================================================================
                              RA TECH
                           RA BOT NEO v2.0
                 Cute High-Tech ESP32-S3 Desk Companion
  ======================================================================

  Hardware
  ----------------------------------------------------------------------
  ESP32-S3 DevKit
  SSD1306 128x64 I2C OLED
  TTP223 capacitive touch sensor
  1x WS2812 / NeoPixel
  Passive buzzer

  Wiring
  ----------------------------------------------------------------------
  OLED SDA        -> GPIO 8
  OLED SCL        -> GPIO 9
  TTP223 OUT      -> GPIO 4
  WS2812 DIN      -> GPIO 5
  Buzzer +        -> GPIO 6
  All grounds     -> Common GND

  Touch gestures
  ----------------------------------------------------------------------
  1 quick tap     -> Happy hello
  2 quick taps    -> Excited bounce
  3 quick taps    -> Confused/funny reaction
  4 quick taps    -> Suspicious glitch/side-eye
  5+ quick taps   -> Crazy funny mode

  Hold 0.6-2 sec  -> Squishy pet reaction
  Hold 2-5 sec    -> UWU / cuddle reaction
  Hold 5+ sec     -> Heart eyes / maximum affection

  No touch for 15 seconds -> Sleep
  Touch while sleeping    -> Wake + start enjoying the pet

  Required Arduino libraries
  ----------------------------------------------------------------------
  Adafruit GFX Library
  Adafruit SSD1306
  Adafruit NeoPixel

  ======================================================================
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>

// ======================================================================
// PIN CONFIGURATION
// ======================================================================

#define OLED_SDA        8
#define OLED_SCL        9
#define TOUCH_PIN       4
#define WS2812_PIN      5
#define BUZZER_PIN      6

// ======================================================================
// OLED CONFIGURATION
// ======================================================================

#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64
#define OLED_RESET      -1
#define OLED_ADDRESS    0x3C

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

// ======================================================================
// WS2812 CONFIGURATION
// ======================================================================

#define LED_COUNT 1

Adafruit_NeoPixel pixel(
  LED_COUNT,
  WS2812_PIN,
  NEO_GRB + NEO_KHZ800
);

// ======================================================================
// ROBOT MODE
// ======================================================================

enum RobotMode {
  MODE_IDLE,
  MODE_HOLDING,
  MODE_SLEEPING
};

RobotMode robotMode = MODE_IDLE;

// ======================================================================
// TIMING
// ======================================================================

const unsigned long SLEEP_AFTER_MS       = 15000UL;
const unsigned long MULTI_TAP_WINDOW_MS  = 420UL;
const unsigned long HOLD_START_MS        = 600UL;
const unsigned long HOLD_UWU_MS          = 2000UL;
const unsigned long HOLD_LOVE_MS         = 5000UL;

unsigned long lastInteraction   = 0;
unsigned long lastFrame         = 0;
unsigned long lastBlink         = 0;
unsigned long nextBlink         = 2600;
unsigned long lastIdleAction    = 0;
unsigned long nextIdleAction    = 1800;

unsigned long touchStartTime    = 0;
unsigned long lastTapRelease    = 0;
unsigned long lastHoldFrame     = 0;

// ======================================================================
// TOUCH STATE
// ======================================================================

bool touchNow       = false;
bool touchPrevious  = false;
bool holdRecognized = false;
bool wokeFromSleep  = false;

uint8_t tapCount = 0;

// ======================================================================
// EYE ENGINE
// ======================================================================

float eyeX        = 0.0f;
float eyeY        = 0.0f;
float targetEyeX  = 0.0f;
float targetEyeY  = 0.0f;
float breathePhase = 0.0f;
float ledPhase     = 0.0f;

const int LEFT_EYE_X   = 18;
const int RIGHT_EYE_X  = 74;
const int BASE_EYE_Y   = 14;
const int EYE_WIDTH    = 36;
const int EYE_HEIGHT   = 36;

// ======================================================================
// GENERAL HELPERS
// ======================================================================

void setLED(uint8_t r, uint8_t g, uint8_t b) {
  pixel.setPixelColor(0, pixel.Color(r, g, b));
  pixel.show();
}

void beep(uint16_t frequency, uint16_t durationMs) {
  tone(BUZZER_PIN, frequency, durationMs);
}

void resetEyeTargets() {
  targetEyeX = 0;
  targetEyeY = 0;
}

void drawTechCorners() {
  display.drawLine(2, 8, 2, 2, SSD1306_WHITE);
  display.drawLine(2, 2, 9, 2, SSD1306_WHITE);

  display.drawLine(118, 2, 125, 2, SSD1306_WHITE);
  display.drawLine(125, 2, 125, 8, SSD1306_WHITE);

  display.drawLine(2, 55, 2, 61, SSD1306_WHITE);
  display.drawLine(2, 61, 9, 61, SSD1306_WHITE);

  display.drawLine(118, 61, 125, 61, SSD1306_WHITE);
  display.drawLine(125, 55, 125, 61, SSD1306_WHITE);
}

void drawCenterStatusDot() {
  display.fillCircle(64, 59, 1, SSD1306_WHITE);
}

// ======================================================================
// SOUND PERSONALITY
// ======================================================================

void soundBoot() {
  beep(520, 55);  delay(70);
  beep(760, 55);  delay(70);
  beep(1030, 60); delay(75);
  beep(1380, 110);
}

void soundHappy() {
  beep(900, 55);  delay(65);
  beep(1200, 55); delay(65);
  beep(1550, 95);
}

void soundExcited() {
  beep(950, 45);  delay(55);
  beep(1250, 45); delay(55);
  beep(1550, 45); delay(55);
  beep(1850, 90);
}

void soundLove() {
  beep(1150, 55); delay(70);
  beep(1450, 60); delay(75);
  beep(1800, 100);
}

void soundConfused() {
  beep(760, 60); delay(80);
  beep(520, 90);
}

void soundWake() {
  beep(430, 55);  delay(70);
  beep(700, 55);  delay(70);
  beep(1050, 65); delay(80);
  beep(1450, 110);
}

void soundSleep() {
  beep(850, 70); delay(90);
  beep(620, 85); delay(100);
  beep(390, 130);
}

void soundBoop() {
  beep(1500, 45);
}

void soundCrazy() {
  const int notes[] = {900, 1500, 700, 1800, 1100, 2000};
  for (int i = 0; i < 6; i++) {
    beep(notes[i], 50);
    delay(62);
  }
}

// ======================================================================
// LED PERSONALITY
// ======================================================================

void updateIdleLED() {
  ledPhase += 0.045f;

  if (ledPhase > 6.28318f) {
    ledPhase = 0.0f;
  }

  float p = (sin(ledPhase) + 1.0f) * 0.5f;

  uint8_t g = 4 + (uint8_t)(p * 18.0f);
  uint8_t b = 12 + (uint8_t)(p * 55.0f);

  setLED(0, g, b);
}

// ======================================================================
// BASE CUTE EYES
// ======================================================================

void drawCuteEye(int x, int y, int w, int h, bool winkShine = true) {
  int radius = max(3, min(w, h) / 3);

  display.fillRoundRect(
    x, y, w, h,
    radius,
    SSD1306_WHITE
  );

  if (winkShine && h > 15) {
    // Dark glossy pupil zone.
    display.fillCircle(
      x + w - 11,
      y + 10,
      5,
      SSD1306_BLACK
    );

    // Small white sparkle inside the pupil.
    display.fillCircle(
      x + w - 13,
      y + 7,
      1,
      SSD1306_WHITE
    );
  }
}

void updateEyeMovement() {
  eyeX += (targetEyeX - eyeX) * 0.17f;
  eyeY += (targetEyeY - eyeY) * 0.17f;

  if (fabs(targetEyeX - eyeX) < 0.05f) eyeX = targetEyeX;
  if (fabs(targetEyeY - eyeY) < 0.05f) eyeY = targetEyeY;
}

void drawNormalFace() {
  display.clearDisplay();

  drawTechCorners();

  int floatY = (int)(sin(breathePhase) * 1.3f);
  int ex = (int)eyeX;
  int ey = (int)eyeY + floatY;

  drawCuteEye(
    LEFT_EYE_X + ex,
    BASE_EYE_Y + ey,
    EYE_WIDTH,
    EYE_HEIGHT
  );

  drawCuteEye(
    RIGHT_EYE_X + ex,
    BASE_EYE_Y + ey,
    EYE_WIDTH,
    EYE_HEIGHT
  );

  drawCenterStatusDot();

  display.display();
}

// ======================================================================
// BLINKING
// ======================================================================

void drawBlinkFrame(int h) {
  display.clearDisplay();
  drawTechCorners();

  int y =
    BASE_EYE_Y +
    EYE_HEIGHT / 2 -
    h / 2;

  display.fillRoundRect(
    LEFT_EYE_X + (int)eyeX,
    y + (int)eyeY,
    EYE_WIDTH,
    h,
    6,
    SSD1306_WHITE
  );

  display.fillRoundRect(
    RIGHT_EYE_X + (int)eyeX,
    y + (int)eyeY,
    EYE_WIDTH,
    h,
    6,
    SSD1306_WHITE
  );

  drawCenterStatusDot();
  display.display();
}

void blinkEyes() {
  for (int h = EYE_HEIGHT; h >= 3; h -= 6) {
    drawBlinkFrame(h);
    delay(17);
  }

  delay(45);

  for (int h = 3; h <= EYE_HEIGHT; h += 6) {
    drawBlinkFrame(h);
    delay(17);
  }

  drawNormalFace();
}

void cuteDoubleBlink() {
  blinkEyes();
  delay(95);
  blinkEyes();
}

// ======================================================================
// LOOK / SCAN ANIMATIONS
// ======================================================================

void smoothLook(float x, float y, uint16_t moveMs, uint16_t holdMs = 0) {
  targetEyeX = x;
  targetEyeY = y;

  unsigned long start = millis();

  while (millis() - start < moveMs) {
    updateEyeMovement();
    breathePhase += 0.045f;
    drawNormalFace();
    delay(16);
  }

  if (holdMs > 0) {
    delay(holdMs);
  }
}

void lookHome() {
  smoothLook(0, 0, 250);
}

void curiousScan() {
  setLED(0, 24, 85);

  smoothLook(-8, 0, 280, 80);
  smoothLook(8, 0, 420, 100);
  smoothLook(0, -4, 280, 80);
  smoothLook(-4, 3, 240, 70);
  lookHome();
}

void quickLookAround() {
  smoothLook(-8, 0, 180, 40);
  smoothLook(8, 0, 220, 40);
  lookHome();
}

// ======================================================================
// EXPRESSIONS
// ======================================================================

void happyFace(bool playSound = false) {
  setLED(0, 65, 28);

  display.clearDisplay();
  drawTechCorners();

  // Two curved smiling eyes.
  display.drawRoundRect(17, 24, 39, 21, 10, SSD1306_WHITE);
  display.fillRect(14, 34, 46, 18, SSD1306_BLACK);

  display.drawRoundRect(72, 24, 39, 21, 10, SSD1306_WHITE);
  display.fillRect(69, 34, 46, 18, SSD1306_BLACK);

  // Tiny cheek pixels.
  display.fillCircle(12, 43, 1, SSD1306_WHITE);
  display.fillCircle(116, 43, 1, SSD1306_WHITE);

  drawCenterStatusDot();
  display.display();

  if (playSound) soundHappy();
}

void uwuFace(bool playSound = false) {
  setLED(55, 0, 42);

  display.clearDisplay();
  drawTechCorners();

  // >w< style eyes.
  display.drawLine(18, 31, 31, 39, SSD1306_WHITE);
  display.drawLine(31, 39, 45, 30, SSD1306_WHITE);

  display.drawLine(83, 30, 97, 39, SSD1306_WHITE);
  display.drawLine(97, 39, 111, 31, SSD1306_WHITE);

  // Tiny "w" mouth.
  display.drawLine(60, 49, 63, 52, SSD1306_WHITE);
  display.drawLine(63, 52, 64, 50, SSD1306_WHITE);
  display.drawLine(64, 50, 65, 52, SSD1306_WHITE);
  display.drawLine(65, 52, 68, 49, SSD1306_WHITE);

  display.display();

  if (playSound) {
    beep(1380, 55);
    delay(65);
    beep(1660, 75);
  }
}

void drawHeart(int cx, int cy, int scale = 1) {
  int r = 6 * scale;
  int dx = 5 * scale;

  display.fillCircle(cx - dx, cy - 4 * scale, r, SSD1306_WHITE);
  display.fillCircle(cx + dx, cy - 4 * scale, r, SSD1306_WHITE);

  display.fillTriangle(
    cx - 11 * scale, cy,
    cx + 11 * scale, cy,
    cx, cy + 15 * scale,
    SSD1306_WHITE
  );
}

void loveFace(bool playSound = false) {
  setLED(100, 0, 26);

  for (int pulse = 0; pulse < 3; pulse++) {
    display.clearDisplay();
    drawTechCorners();

    int lift = (pulse % 2 == 0) ? 0 : -2;

    drawHeart(38, 27 + lift);
    drawHeart(90, 27 + lift);

    // Tiny center heart-like mouth.
    display.drawLine(61, 52, 64, 55, SSD1306_WHITE);
    display.drawLine(64, 55, 67, 52, SSD1306_WHITE);

    display.display();
    delay(135);
  }

  if (playSound) soundLove();
}

void squishyFace(bool altFrame = false) {
  setLED(48, 5, 68);

  display.clearDisplay();
  drawTechCorners();

  int h = altFrame ? 11 : 15;
  int y = altFrame ? 29 : 27;

  display.fillRoundRect(17, y, 39, h, 7, SSD1306_WHITE);
  display.fillRoundRect(72, y, 39, h, 7, SSD1306_WHITE);

  // Tiny smile.
  display.drawPixel(62, 49, SSD1306_WHITE);
  display.drawPixel(63, 50, SSD1306_WHITE);
  display.drawPixel(64, 51, SSD1306_WHITE);
  display.drawPixel(65, 50, SSD1306_WHITE);
  display.drawPixel(66, 49, SSD1306_WHITE);

  display.display();
}

void excitedFace(bool playSound = false) {
  setLED(8, 82, 70);

  for (int frame = 0; frame < 6; frame++) {
    display.clearDisplay();
    drawTechCorners();

    int jump = (frame % 2 == 0) ? -3 : 1;
    int squish = (frame % 2 == 0) ? 0 : 4;

    display.fillRoundRect(
      17,
      13 + jump + squish / 2,
      39,
      38 - squish,
      12,
      SSD1306_WHITE
    );

    display.fillRoundRect(
      72,
      13 + jump + squish / 2,
      39,
      38 - squish,
      12,
      SSD1306_WHITE
    );

    display.fillCircle(43, 25 + jump, 5, SSD1306_BLACK);
    display.fillCircle(98, 25 + jump, 5, SSD1306_BLACK);

    display.fillCircle(41, 23 + jump, 1, SSD1306_WHITE);
    display.fillCircle(96, 23 + jump, 1, SSD1306_WHITE);

    display.display();
    delay(70);
  }

  if (playSound) soundExcited();
}

void surprisedFace(bool playSound = false) {
  setLED(75, 28, 0);

  display.clearDisplay();
  drawTechCorners();

  display.fillCircle(38, 29, 17, SSD1306_WHITE);
  display.fillCircle(90, 29, 17, SSD1306_WHITE);

  display.fillCircle(38, 29, 5, SSD1306_BLACK);
  display.fillCircle(90, 29, 5, SSD1306_BLACK);

  display.fillCircle(36, 27, 1, SSD1306_WHITE);
  display.fillCircle(88, 27, 1, SSD1306_WHITE);

  display.drawCircle(64, 54, 3, SSD1306_WHITE);

  display.display();

  if (playSound) {
    beep(720, 55);
    delay(65);
    beep(1350, 100);
  }
}

void confusedFace(bool playSound = false) {
  setLED(48, 24, 0);

  display.clearDisplay();
  drawTechCorners();

  display.fillRoundRect(17, 16, 39, 36, 11, SSD1306_WHITE);
  display.fillRoundRect(80, 25, 29, 25, 8, SSD1306_WHITE);

  display.fillCircle(43, 29, 4, SSD1306_BLACK);
  display.fillCircle(99, 34, 3, SSD1306_BLACK);

  // Tiny tilted mouth.
  display.drawLine(60, 53, 67, 50, SSD1306_WHITE);

  display.display();

  if (playSound) soundConfused();
}

void sideEyeFace() {
  setLED(0, 20, 55);

  display.clearDisplay();
  drawTechCorners();

  display.fillRoundRect(17, 18, 37, 30, 10, SSD1306_WHITE);
  display.fillRoundRect(74, 18, 37, 30, 10, SSD1306_WHITE);

  display.fillCircle(27, 32, 5, SSD1306_BLACK);
  display.fillCircle(84, 32, 5, SSD1306_BLACK);

  display.display();
}

void processingFace() {
  setLED(0, 28, 78);

  for (int step = 0; step < 5; step++) {
    display.clearDisplay();
    drawTechCorners();

    display.fillRoundRect(21, 21, 31, 28, 8, SSD1306_WHITE);
    display.fillRoundRect(76, 21, 31, 28, 8, SSD1306_WHITE);

    display.fillCircle(30 + step * 4, 34, 3, SSD1306_BLACK);
    display.fillCircle(85 + step * 4, 34, 3, SSD1306_BLACK);

    for (int d = 0; d < 3; d++) {
      int x = 55 + d * 9;
      int filled = (d == (step % 3));
      if (filled) {
        display.fillCircle(x, 57, 2, SSD1306_WHITE);
      } else {
        display.drawCircle(x, 57, 2, SSD1306_WHITE);
      }
    }

    display.display();
    delay(100);
  }
}

void digitalGlitch(uint8_t frames = 4) {
  setLED(0, 55, 95);

  for (uint8_t i = 0; i < frames; i++) {
    display.clearDisplay();

    int offset1 = random(-4, 5);
    int offset2 = random(-4, 5);

    display.fillRoundRect(
      LEFT_EYE_X + offset1,
      BASE_EYE_Y,
      EYE_WIDTH,
      EYE_HEIGHT,
      10,
      SSD1306_WHITE
    );

    display.fillRoundRect(
      RIGHT_EYE_X + offset2,
      BASE_EYE_Y,
      EYE_WIDTH,
      EYE_HEIGHT,
      10,
      SSD1306_WHITE
    );

    display.drawFastHLine(
      random(0, 18),
      random(8, 56),
      random(70, 125),
      SSD1306_WHITE
    );

    display.display();
    delay(45);
  }
}

void tinyBoopFace() {
  setLED(25, 40, 95);

  display.clearDisplay();
  drawTechCorners();

  display.fillCircle(38, 30, 4, SSD1306_WHITE);
  display.fillCircle(90, 30, 4, SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(62, 47);
  display.print("!");

  display.display();

  soundBoop();
  delay(210);

  surprisedFace(false);
  delay(280);
}

void crazyFunnyFace() {
  soundCrazy();

  for (int i = 0; i < 2; i++) {
    digitalGlitch(3);
    sideEyeFace();
    delay(110);
    surprisedFace(false);
    delay(110);
    uwuFace(false);
    delay(110);
  }

  // @_@ style finale.
  display.clearDisplay();
  drawTechCorners();

  display.setTextSize(2);
  display.setCursor(23, 22);
  display.print("@");
  display.setCursor(84, 22);
  display.print("@");

  display.setTextSize(1);
  display.setCursor(58, 48);
  display.print("w");

  display.display();
  delay(450);

  happyFace(false);
  delay(300);
}

// ======================================================================
// TAP REACTIONS
// ======================================================================

void reactionSingleTap() {
  // Cute tiny "boop" first, then happy hello.
  tinyBoopFace();
  happyFace(true);
  delay(450);
}

void reactionDoubleTap() {
  excitedFace(true);
  quickLookAround();
  happyFace(false);
  delay(320);
}

void reactionTripleTap() {
  confusedFace(true);
  delay(430);
  sideEyeFace();
  delay(430);
  happyFace(false);
  delay(280);
}

void reactionFourTap() {
  sideEyeFace();
  delay(260);
  digitalGlitch(5);
  processingFace();
  surprisedFace(false);
  delay(320);
}

void reactionFivePlusTap() {
  crazyFunnyFace();
}

// ======================================================================
// HOLD / PETTING REACTIONS
// ======================================================================

void showHoldingReaction(unsigned long heldMs) {
  if (millis() - lastHoldFrame < 180) {
    return;
  }

  lastHoldFrame = millis();

  if (heldMs < HOLD_UWU_MS) {
    bool alt = ((heldMs / 250) % 2) != 0;
    squishyFace(alt);

    if ((heldMs / 500) != ((heldMs - 180) / 500)) {
      beep(1180 + random(0, 180), 35);
    }
  }
  else if (heldMs < HOLD_LOVE_MS) {
    // Alternate between UWU and happy while being petted.
    if (((heldMs / 550) % 2) == 0) {
      uwuFace(false);
    } else {
      happyFace(false);
    }

    if ((heldMs / 700) != ((heldMs - 180) / 700)) {
      beep(1400 + random(0, 250), 40);
    }
  }
  else {
    // Maximum affection.
    loveFace(false);

    if ((heldMs / 900) != ((heldMs - 180) / 900)) {
      beep(1700 + random(0, 220), 45);
    }
  }
}

void finishHoldReaction(unsigned long heldMs) {
  if (heldMs < HOLD_UWU_MS) {
    squishyFace(false);
    beep(1250, 55);
    delay(250);
    happyFace(false);
    delay(280);
  }
  else if (heldMs < HOLD_LOVE_MS) {
    uwuFace(true);
    delay(330);
    happyFace(false);
    delay(280);
  }
  else {
    loveFace(true);
    delay(380);
    excitedFace(false);
    delay(260);
    happyFace(false);
    delay(280);
  }

  robotMode = MODE_IDLE;
  resetEyeTargets();

  lastInteraction = millis();
  lastIdleAction = millis();
  lastBlink = millis();
}

// ======================================================================
// RANDOM IDLE PERSONALITY
// ======================================================================

void performIdleAction() {
  int action = random(0, 15);

  switch (action) {
    case 0:
      smoothLook(-8, 0, 260, 100);
      lookHome();
      break;

    case 1:
      smoothLook(8, 0, 260, 100);
      lookHome();
      break;

    case 2:
      smoothLook(0, -5, 260, 100);
      lookHome();
      break;

    case 3:
      curiousScan();
      break;

    case 4:
      blinkEyes();
      break;

    case 5:
      cuteDoubleBlink();
      break;

    case 6:
      sideEyeFace();
      delay(500);
      break;

    case 7:
      confusedFace(false);
      delay(500);
      break;

    case 8:
      uwuFace(false);
      delay(500);
      break;

    case 9:
      happyFace(false);
      delay(480);
      break;

    case 10:
      processingFace();
      break;

    case 11:
      digitalGlitch(3);
      break;

    case 12:
      surprisedFace(false);
      delay(380);
      break;

    case 13:
      quickLookAround();
      break;

    default:
      squishyFace(false);
      delay(330);
      happyFace(false);
      delay(300);
      break;
  }

  resetEyeTargets();
}

// ======================================================================
// SLEEP / WAKE
// ======================================================================

void drawSleepingFace(uint8_t zOffset = 0) {
  display.clearDisplay();

  // Closed soft eyes.
  display.drawLine(18, 32, 54, 32, SSD1306_WHITE);
  display.drawLine(74, 32, 110, 32, SSD1306_WHITE);

  display.setTextSize(1);

  display.setCursor(104, 15 + zOffset);
  display.print("z");

  display.setCursor(112, 8 + zOffset);
  display.print("Z");

  display.setCursor(120, 1 + zOffset);
  display.print("Z");

  // Sleeping core light.
  display.drawCircle(64, 55, 2, SSD1306_WHITE);

  display.display();
}

void sleepRobot() {
  robotMode = MODE_SLEEPING;
  tapCount = 0;
  holdRecognized = false;

  setLED(2, 0, 8);

  // Getting sleepy: droop, reopen, droop deeper.
  for (int cycle = 0; cycle < 2; cycle++) {
    display.clearDisplay();
    drawTechCorners();

    int h = (cycle == 0) ? 10 : 6;
    int y = BASE_EYE_Y + EYE_HEIGHT / 2 - h / 2;

    display.fillRoundRect(LEFT_EYE_X, y, EYE_WIDTH, h, 4, SSD1306_WHITE);
    display.fillRoundRect(RIGHT_EYE_X, y, EYE_WIDTH, h, 4, SSD1306_WHITE);

    display.display();
    delay(320);

    if (cycle == 0) {
      drawNormalFace();
      delay(250);
    }
  }

  // Cute yawn.
  display.clearDisplay();

  display.fillRoundRect(18, 29, 36, 5, 3, SSD1306_WHITE);
  display.fillRoundRect(74, 29, 36, 5, 3, SSD1306_WHITE);
  display.drawCircle(64, 51, 5, SSD1306_WHITE);

  display.display();
  delay(480);

  soundSleep();

  // Slowly close.
  for (int h = EYE_HEIGHT; h >= 3; h -= 3) {
    drawBlinkFrame(h);
    delay(28);
  }

  drawSleepingFace(0);
}

void wakeRobot() {
  robotMode = MODE_HOLDING;
  wokeFromSleep = true;
  holdRecognized = true;

  tapCount = 0;

  setLED(0, 42, 100);
  soundWake();

  // Sci-fi wake scan.
  for (int x = 0; x < 128; x += 8) {
    display.clearDisplay();

    display.drawFastVLine(
      x,
      8,
      48,
      SSD1306_WHITE
    );

    display.drawPixel((x + 30) % 128, 17, SSD1306_WHITE);
    display.drawPixel((x + 70) % 128, 44, SSD1306_WHITE);

    display.display();
    delay(16);
  }

  // Eyes opening.
  for (int h = 3; h <= EYE_HEIGHT; h += 3) {
    drawBlinkFrame(h);
    delay(26);
  }

  surprisedFace(false);
  delay(250);

  blinkEyes();

  happyFace(true);
  delay(320);

  // The waking touch becomes petting immediately.
  touchStartTime = millis();
  lastHoldFrame = 0;
  lastInteraction = millis();
}

// ======================================================================
// BOOT ANIMATION
// ======================================================================

void bootAnimation() {
  setLED(0, 0, 15);

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // Scan line.
  for (int y = 0; y < 64; y += 3) {
    display.clearDisplay();
    display.drawFastHLine(0, y, 128, SSD1306_WHITE);

    // A few tech dots make the scan feel more digital.
    display.drawPixel((y * 2) % 128, 12, SSD1306_WHITE);
    display.drawPixel((y * 3) % 128, 49, SSD1306_WHITE);

    display.display();
    delay(11);
  }

  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(43, 7);
  display.print("RA TECH");

  display.setCursor(28, 22);
  display.print("NEURAL CORE");

  display.setCursor(36, 37);
  display.print("ONLINE");

  display.setCursor(34, 51);
  display.print("RA BOT NEO");

  display.display();

  soundBoot();
  delay(650);

  // Loading bar.
  for (int p = 0; p <= 100; p += 10) {
    display.clearDisplay();

    drawTechCorners();

    display.setTextSize(1);
    display.setCursor(43, 14);
    display.print("RA BOT");

    display.drawRect(18, 33, 92, 10, SSD1306_WHITE);

    int bar = map(p, 0, 100, 0, 88);

    display.fillRect(20, 35, bar, 6, SSD1306_WHITE);

    display.setCursor(52, 50);
    display.print(p);
    display.print("%");

    display.display();
    delay(55);
  }

  delay(180);

  // Wake eyes from a thin digital line.
  for (int h = 3; h <= EYE_HEIGHT; h += 3) {
    drawBlinkFrame(h);
    delay(25);
  }

  delay(180);

  curiousScan();
  cuteDoubleBlink();
  happyFace(true);
  delay(480);

  resetEyeTargets();
}

// ======================================================================
// TAP PROCESSOR
// ======================================================================

void processTapSequence() {
  if (tapCount == 0) return;

  uint8_t count = tapCount;
  tapCount = 0;

  switch (count) {
    case 1:
      reactionSingleTap();
      break;

    case 2:
      reactionDoubleTap();
      break;

    case 3:
      reactionTripleTap();
      break;

    case 4:
      reactionFourTap();
      break;

    default:
      reactionFivePlusTap();
      break;
  }

  robotMode = MODE_IDLE;
  resetEyeTargets();

  lastInteraction = millis();
  lastIdleAction = millis();
  lastBlink = millis();
}

// ======================================================================
// SETUP
// ======================================================================

void setup() {
  Serial.begin(115200);

  pinMode(TOUCH_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  Wire.begin(OLED_SDA, OLED_SCL);

  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        OLED_ADDRESS
      )) {
    Serial.println("ERROR: SSD1306 OLED not found.");

    while (true) {
      delay(100);
    }
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.display();

  pixel.begin();
  pixel.clear();
  pixel.show();

  randomSeed((uint32_t)micros());

  bootAnimation();

  robotMode = MODE_IDLE;

  lastInteraction = millis();
  lastFrame = millis();
  lastBlink = millis();
  lastIdleAction = millis();

  nextBlink = random(2000, 4500);
  nextIdleAction = random(1200, 2600);
}

// ======================================================================
// MAIN LOOP
// ======================================================================

void loop() {
  unsigned long now = millis();

  // TTP223 in momentary, active-HIGH mode. Reject short electrical glitches.
  static bool rawPrevious = false;
  static bool stableTouch = false;
  static unsigned long changedAt = 0;
  bool rawTouch = digitalRead(TOUCH_PIN) == HIGH;
  if (rawTouch != rawPrevious) {
    rawPrevious = rawTouch;
    changedAt = now;
  }
  if (now - changedAt >= 25) stableTouch = rawTouch;
  touchNow = stableTouch;

  // --------------------------------------------------------------------
  // TOUCH START
  // --------------------------------------------------------------------

  if (touchNow && !touchPrevious) {
    lastInteraction = now;

    // Sleeping touch gets priority.
    if (robotMode == MODE_SLEEPING) {
      wakeRobot();

      // wakeRobot() contains animations, so refresh the loop timestamp
      // before measuring the waking touch duration.
      now = millis();
      // Preserve the press edge even if the finger was released during wake.
      touchPrevious = true;
      touchNow = digitalRead(TOUCH_PIN) == HIGH;
      stableTouch = rawPrevious = touchNow;
      changedAt = now;
    }
    else {
      touchStartTime = now;
      holdRecognized = false;
      wokeFromSleep = false;

      // A tiny visual acknowledgement without committing to a gesture yet.
      setLED(8, 32, 85);
    }
  }

  // --------------------------------------------------------------------
  // TOUCH HELD
  // --------------------------------------------------------------------

  if (touchNow && robotMode != MODE_SLEEPING) {
    lastInteraction = now;

    unsigned long heldMs = now - touchStartTime;

    if (!wokeFromSleep && !holdRecognized && heldMs >= HOLD_START_MS) {
      holdRecognized = true;
      robotMode = MODE_HOLDING;

      // If a hold starts, cancel pending tap sequence.
      tapCount = 0;
    }

    if (robotMode == MODE_HOLDING && holdRecognized) {
      showHoldingReaction(heldMs);
    }
  }

  // --------------------------------------------------------------------
  // TOUCH RELEASE
  // --------------------------------------------------------------------

  if (!touchNow && touchPrevious) {
    unsigned long heldMs = now - touchStartTime;
    lastInteraction = now;

    if (robotMode == MODE_HOLDING && holdRecognized) {
      // If this was the wake touch, use actual time held since wake.
      finishHoldReaction(heldMs);
      now = millis(); // animations updated lastInteraction; avoid unsigned underflow

      wokeFromSleep = false;
      holdRecognized = false;
    }
    else if (robotMode != MODE_SLEEPING) {
      // Quick touch -> one tap in the multi-tap sequence.
      if (heldMs < HOLD_START_MS) {
        if (tapCount < 10) {
          tapCount++;
        }

        lastTapRelease = now;
      }
    }
  }

  touchPrevious = touchNow;

  // --------------------------------------------------------------------
  // MULTI-TAP DECISION
  // --------------------------------------------------------------------

  if (
    tapCount > 0 &&
    !touchNow &&
    robotMode == MODE_IDLE &&
    now - lastTapRelease >= MULTI_TAP_WINDOW_MS
  ) {
    processTapSequence();
    now = millis();
  }

  // --------------------------------------------------------------------
  // SLEEPING
  // --------------------------------------------------------------------

  if (robotMode == MODE_SLEEPING) {
    // Small breathing motion in the Z letters.
    static unsigned long lastSleepAnim = 0;
    static bool sleepFlip = false;

    if (now - lastSleepAnim > 850) {
      sleepFlip = !sleepFlip;
      drawSleepingFace(sleepFlip ? 1 : 0);
      lastSleepAnim = now;
    }

    delay(10);
    return;
  }

  // --------------------------------------------------------------------
  // HOLDING
  // --------------------------------------------------------------------

  if (robotMode == MODE_HOLDING) {
    delay(10);
    return;
  }

  // --------------------------------------------------------------------
  // IDLE FRAME LOOP
  // --------------------------------------------------------------------

  if (now - lastFrame >= 32) {
    lastFrame = now;

    breathePhase += 0.05f;

    if (breathePhase > 6.28318f) {
      breathePhase = 0.0f;
    }

    updateEyeMovement();
    updateIdleLED();

    // Do not overwrite the screen while waiting to see whether
    // another tap will arrive.
    if (tapCount == 0 && !touchNow) {
      drawNormalFace();
    }
  }

  // --------------------------------------------------------------------
  // NATURAL BLINK
  // --------------------------------------------------------------------

  if (
    tapCount == 0 &&
    !touchNow &&
    now - lastBlink >= nextBlink
  ) {
    if (random(0, 10) < 3) {
      cuteDoubleBlink();
    }
    else {
      blinkEyes();
    }

    lastBlink = millis();
    nextBlink = random(1800, 4700);

    now = millis();
  }

  // --------------------------------------------------------------------
  // RANDOM CUTE / HIGH-TECH IDLE ACTION
  // --------------------------------------------------------------------

  if (
    tapCount == 0 &&
    !touchNow &&
    now - lastIdleAction >= nextIdleAction
  ) {
    performIdleAction();

    lastIdleAction = millis();
    nextIdleAction = random(1200, 2800);

    now = millis();
  }

  // --------------------------------------------------------------------
  // AUTO SLEEP AFTER 15 SECONDS WITHOUT HUMAN INTERACTION
  // --------------------------------------------------------------------

  if (
    !touchNow &&
    tapCount == 0 &&
    now - lastInteraction >= SLEEP_AFTER_MS
  ) {
    sleepRobot();
  }

  delay(8);
}
