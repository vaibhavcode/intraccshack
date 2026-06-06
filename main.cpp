
#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>
#include <BluetoothSerial.h>

#define LED 2

BluetoothSerial BT;
MPU6050 mpu;

float prevAx = 0;
int thresh = 8;
int count = 0;


void setup() {
    Serial.begin(115200);
    Wire.begin(21, 22);
    mpu.initialize();
    pinMode(LED, OUTPUT);
    BT.begin("ESP32-MPU"); 
    Serial.println("Bluetooth started, pair with ESP32-MPU");
}

void loop() {
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getAcceleration(&ax, &ay, &az);
    mpu.getRotation(&gx, &gy, &gz);

    float Ax = ax / 16384.0f * 9.81f;
    Serial.println(Ax);

    if ((Ax - prevAx) > thresh) {
        digitalWrite(LED, HIGH);
        count+=1;
        BT.print("SCROLL DETECTED: ");
        BT.println(count);
        delay(500);
        digitalWrite(LED, LOW);
    }

    prevAx = Ax;
    delay(50);
}
