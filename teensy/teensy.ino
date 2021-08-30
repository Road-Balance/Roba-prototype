#include "src/ODriveArduino/ODriveArduino.h"
#include <HardwareSerial.h>

#define ODRIVE_COUNT 2
#define ODRIVE_CALIBRATION_COUNT 1
#define ODRIVE_AXIS_PER_BOARD 2
#define MOTOR_1 0
#define MOTOR_2 1
HardwareSerial &odriveSerialFront = Serial1;
HardwareSerial &odriveSerialBack = Serial2;
ODriveArduino odriveFront(odriveSerialFront);
ODriveArduino odriveBack(odriveSerialBack);
int odriveCalibration[ODRIVE_CALIBRATION_COUNT] = {
    ODriveArduino::AXIS_STATE_CLOSED_LOOP_CONTROL};
// int odriveCalibration[ODRIVE_CALIBRATION_COUNT] =
// {ODriveArduino::AXIS_STATE_MOTOR_CALIBRATION,
// ODriveArduino::AXIS_STATE_ENCODER_OFFSET_CALIBRATION,
// ODriveArduino::AXIS_STATE_CLOSED_LOOP_CONTROL}; ODriveArduino
// odrive[ODRIVE_COUNT] = {odriveFront, odriveBack};
float left, right;

void setup() {
  Serial.begin(115200);
  odriveSerialFront.begin(115200);
  odriveSerialBack.begin(115200);
  Serial.println("init start");
  /*
  for (int o = 0; o < ODRIVE_COUNT; o++) {
      for (int a = 0; a < ODRIVE_AXIS_PER_BOARD; a++) {
          for (int c = 0; c < ODRIVE_CALIBRATION_COUNT; c++) {
              odrive[o].run_state(a, odriveCalibration[c], true);
          }
      }
  }
  */

  Serial.println("cali start");
  odriveFront.run_state(MOTOR_1, ODriveArduino::AXIS_STATE_CLOSED_LOOP_CONTROL,
                        false, 0);
  Serial.println("cali 1");
  odriveSerialFront.flush();
  odriveFront.run_state(MOTOR_2, ODriveArduino::AXIS_STATE_CLOSED_LOOP_CONTROL,
                        false, 0);
  Serial.println("cali 2");
  odriveSerialFront.flush();
  odriveBack.run_state(MOTOR_1, ODriveArduino::AXIS_STATE_CLOSED_LOOP_CONTROL,
                       false, 0);
  Serial.println("cali 3");
  odriveSerialBack.flush();
  odriveBack.run_state(MOTOR_2, ODriveArduino::AXIS_STATE_CLOSED_LOOP_CONTROL,
                       false, 0);
  odriveSerialBack.flush();
  Serial.println("init OK");
}

void loop() {
  byte buf[16];
  byte len;
  Serial.setTimeout(5);
  float axisLeft, axisRight;
  while (true) {
    len = Serial.readBytesUntil(0xFF, (uint8_t *)&buf, 16);
    // buf [1] = a1x, [2] = a1y, [3] = a2x, [4] = a2y
    if (len > 0) {
      axisLeft = (buf[2] - 128) * 0.01f;
      axisRight = (buf[4] - 128) * 0.01f;
      odriveFront.SetVelocity(MOTOR_1, axisLeft);
      odriveSerialFront.flush();
      odriveFront.SetVelocity(MOTOR_2, -axisRight);
      odriveSerialFront.flush();
      odriveBack.SetVelocity(MOTOR_1, axisLeft);
      odriveSerialBack.flush();
      odriveBack.SetVelocity(MOTOR_2, -axisRight);
      odriveSerialBack.flush();
    }
  }
}