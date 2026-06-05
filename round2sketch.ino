#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

#define SDA_PIN D2
#define SCL_PIN D1
#define LED_PIN 2

const float FLICK_THRESHOLD = 0.4;
const int FLICKS_NEEDED = 6;
const unsigned long WINDOW_MS = 5000;
const unsigned long COOLDOWN_MS = 500;
const unsigned long LED_ON_MS = 2000;

int flickCount = 0;
int lastDirection = 0;
unsigned long windowStart = 0;
unsigned long lastFlickAt = 0;
bool windowOpen = false;
bool ledActive = false;
unsigned long ledStartAt = 0;

void ledOn()  { digitalWrite(LED_PIN, LOW); }
void ledOff() { digitalWrite(LED_PIN, HIGH); }

void resetDetection() {
  flickCount = 0;
  lastDirection = 0;
  windowOpen = false;
  windowStart = 0;
  lastFlickAt = 0;
}

void setup() {
  Serial.begin(115200);
  delay(300);
  pinMode(LED_PIN, OUTPUT);
  ledOff();
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);
  mpu.initialize();
  mpu.setSleepEnabled(false);
  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
  mpu.setDLPFMode(MPU6050_DLPF_BW_20);
  Serial.println("=== Reel Scroll Detector Ready ===");
  Serial.printf("Threshold: %.1fg | Flicks: %d | Window: %lus\n\n", FLICK_THRESHOLD, FLICKS_NEEDED, WINDOW_MS / 1000);
}

void loop() {
  unsigned long now = millis();

  if (ledActive) {
    if (now - ledStartAt >= LED_ON_MS) {
      ledOff();
      ledActive = false;
      resetDetection();
      Serial.println("Ready.\n");
    }
    delay(20);
    return;
  }

  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);
  float y = ay / 16384.0;

  int currentDir = 0;
  if      (y >  FLICK_THRESHOLD) currentDir =  1;
  else if (y < -FLICK_THRESHOLD) currentDir = -1;

  if (currentDir != 0 && currentDir != lastDirection) {
    if (now - lastFlickAt >= COOLDOWN_MS) {
      if (!windowOpen) {
        windowOpen = true;
        windowStart = now;
        flickCount = 0;
        Serial.println("Scroll window open.");
      }
      flickCount++;
      lastDirection = currentDir;
      lastFlickAt = now;
      Serial.printf("  Flick %d/%d  y=%.2fg  t=%lums\n", flickCount, FLICKS_NEEDED, y, now - windowStart);
    }
  }

  if (windowOpen && (now - windowStart >= WINDOW_MS)) {
    Serial.printf("Missed. %d/%d flicks in 5s.\n\n", flickCount, FLICKS_NEEDED);
    resetDetection();
  }

  if (windowOpen && flickCount >= FLICKS_NEEDED) {
    Serial.println("SCROLL DETECTED — LED on.\n");
    ledOn();
    ledActive = true;
    ledStartAt = now;
  }

  delay(20);
}
