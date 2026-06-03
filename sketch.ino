#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

const int LED_PIN = 2;

// Adjust based on your Wokwi testing
const float Y_THRESHOLD = 0.3;

const unsigned long REQUIRED_TIME = 5000; // 5 seconds
const unsigned long LED_TIME = 2000;      // 2 seconds

int lastDirection = 0; // -1, 0, +1
bool tracking = false;
unsigned long trackingStart = 0;

bool ledOn = false;
unsigned long ledStart = 0;

void setup() {
  Serial.begin(115200);

  Wire.begin();
  mpu.initialize();

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.println("Ready");
}

void loop() {

  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  float y = ay / 16384.0;

  int currentDirection = 0;

  if (y > Y_THRESHOLD)
    currentDirection = 1;
  else if (y < -Y_THRESHOLD)
    currentDirection = -1;

  // LED timer
  if (ledOn) {
    if (millis() - ledStart >= LED_TIME) {
      ledOn = false;
      digitalWrite(LED_PIN, LOW);

      tracking = false;
      trackingStart = 0;

      Serial.println("Reset");
    }
    return;
  }

  // Detect direction changes
  if (currentDirection != 0 && currentDirection != lastDirection) {

    if (!tracking) {
      tracking = true;
      trackingStart = millis();
      Serial.println("Oscillation started");
    }

    lastDirection = currentDirection;
  }

  // Check if oscillation lasted 5 sec
  if (tracking) {

    unsigned long elapsed = millis() - trackingStart;

    if (elapsed >= REQUIRED_TIME) {

      digitalWrite(LED_PIN, HIGH);
      ledOn = true;
      ledStart = millis();

      Serial.println("SCROLL DETECTED");
    }
  }

  delay(20);
}