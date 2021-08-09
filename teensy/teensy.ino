#include "src/ODriveArduino/ODriveArduino.h"
#include <HardwareSerial.h>

#define ODRIVE_COUNT 2
#define ODRIVE_CALIBRATION_COUNT 3
#define ODRIVE_AXIS_PER_BOARD 2

HardwareSerial& odriveSerialFront = Serial1, odriveSerialBack = Serial2;
ODriveArduino odriveFront(odriveSerialFront), odriveBack(odriveSerialBack);
int odriveCalibration[ODRIVE_CALIBRATION_COUNT] = {ODriveArduino::AXIS_STATE_MOTOR_CALIBRATION, ODriveArduino::AXIS_STATE_ENCODER_OFFSET_CALIBRATION, ODriveArduino::AXIS_STATE_CLOSED_LOOP_CONTROL};
ODriveArduino odrive[ODRIVE_COUNT] = {odriveFront, odriveBack};

void setup() {
    Serial.begin(115200);
    odriveSerialFront.begin(115200);
    odriveSerialBack.begin(115200);
    
    for (int o = 0; o < ODRIVE_COUNT; o++) {
        for (int a = 0; a < ODRIVE_AXIS_PER_BOARD; a++) {
            for (int c = 0; c < ODRIVE_CALIBRATION_COUNT; c++) {
                odrive[o].run_state(a, odriveCalibration[c], true);
            }
        }
    }
}

void loop() {
    byte buf[16];
    byte len;
    while(true) {
        len = Serial.readBytesUntil(0xFF, (uint8_t*)&buf, 16);
        // buf [0] = a1x, [1] = a1y, [2] = a2x, [3] = a2y
        if (len > 0) {
            odriveFront.SetVelocity(0, 10.0f * buf[0]);
        }
    }
}