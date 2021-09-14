#define CONTROL_SERIAL Serial
#define ODRIVE_FRONT_SERIAL Serial1
#define ODRIVE_BACK_SERIAL Serial2
#define ODRIVE_AXIS_STATE_CLOSED_LOOP_CONTROL 8

#define ODRIVE_COUNT 2
#define ODRIVE_AXIS_PER_BOARD 2
#define MOTOR_1 0
#define MOTOR_2 1

#define SERIAL_DELAY 2

int axises[2] = {MOTOR_1, MOTOR_2};

void setup() {
  ODRIVE_FRONT_SERIAL.begin(115200);
  ODRIVE_BACK_SERIAL.begin(115200);
  Serial.begin(115200);
  Serial.println("INIT,UARTOK");

  OdriveSerialSprintf(ODRIVE_FRONT_SERIAL, "sr");
  OdriveSerialSprintf(ODRIVE_BACK_SERIAL, "sr");
  delay(2000);
  Serial.println("INIT,REBOOTOK");

  setOdriveClosedLoop(ODRIVE_FRONT_SERIAL);
  setOdriveClosedLoop(ODRIVE_BACK_SERIAL);
  Serial.println("INIT,SETSTATEOK");
}

void loop() {
  byte buf[16];
  byte len;
  Serial.setTimeout(5);
  float axisLeftY, axisRightY, axisRightX;
  float offsetLeft, offsetRight;
  double vbusf, vbusb;

  Serial.println("INIT,ENTERLOOP");
  while (true) {
    // Serial.println("LOOP");
    len = Serial.readBytesUntil(0xFF, (uint8_t *)&buf, 16);
    // buf [1, 2] = a1x, [3, 4] = a1y, [5, 6] = a2x, [7, 8] = a2y
    if (len > 0) {
      axisLeftY = (buf[2] - 128) * 0.01f;
      axisRightX = (buf[3] - 128) * 0.01f;
      // axisLeftY = ((buf[3] * 255 + buf[4]) - 32767) * 0.0001f;
      // axisRightX = ((buf[5] * 255 + buf[6]) - 32767) * 0.0001f;
      //   axisRightY = ((buf[7] * 255 + buf[8]) - 32767) * 0.0001f;
      Serial.println(axisLeftY);
      Serial.println(axisRightX);
      Serial.println();

      if (axisRightX > 0) {
        offsetLeft = axisRightX;
        offsetRight = 0;
      } else {
        offsetLeft = 0;
        offsetRight = -axisRightX;
      }

      setOdriveVelocity(ODRIVE_FRONT_SERIAL, MOTOR_1, -axisLeftY - offsetLeft);
      // delay(SERIAL_DELAY);
      setOdriveVelocity(ODRIVE_FRONT_SERIAL, MOTOR_2, axisLeftY + offsetRight);
      // delay(SERIAL_DELAY);
      setOdriveVelocity(ODRIVE_BACK_SERIAL, MOTOR_1, -axisLeftY - offsetLeft);
      // delay(SERIAL_DELAY);
      setOdriveVelocity(ODRIVE_BACK_SERIAL, MOTOR_2, axisLeftY + offsetRight);
      // delay(SERIAL_DELAY);
    }

    // readOdriveVariable(ODRIVE_FRONT_SERIAL, "vbus_voltage");
    // delay(SERIAL_DELAY);
    // if (readFloat(ODRIVE_FRONT_SERIAL, &vbusf, 50) > 0) {
    //   if (vbusf < 0) {
    //     Serial.println("ERR,FRONT");
    //     ODRIVE_FRONT_SERIAL.println("sc");
    //     delay(SERIAL_DELAY);
    //     setOdriveClosedLoop(ODRIVE_FRONT_SERIAL);
    //   }
    // } else {
    //   vbusf = -1;
    //   Serial.println("ERR,FRONTREAD");
    // }
    // readOdriveVariable(ODRIVE_BACK_SERIAL, "vbus_voltage");
    // delay(SERIAL_DELAY);
    // if (readFloat(ODRIVE_BACK_SERIAL, &vbusb, 50) > 0) {
    //   if (vbusb < 0) {
    //     Serial.println("ERR,BACK");
    //     ODRIVE_BACK_SERIAL.println("sc");
    //     delay(SERIAL_DELAY);
    //     setOdriveClosedLoop(ODRIVE_BACK_SERIAL);
    //   }
    // } else {
    //   vbusb = -1;
    //   Serial.println("ERR,BACKREAD");
    // }

    // OdriveSerialSprintf(CONTROL_SERIAL, "loop: fvbus=%f bvbus=%f al=%f
    // ar=%f\n",
    //                     vbusf, vbusb, axisLeftY, axisRightX);
  }
}

void setOdriveClosedLoop(Stream &port) {
  for (int a = 0; a < ODRIVE_AXIS_PER_BOARD; a++) {
    setOdriveSetState(port, axises[a], ODRIVE_AXIS_STATE_CLOSED_LOOP_CONTROL);
    delay(SERIAL_DELAY);
  }
}

void setOdriveSetState(Stream &port, int axis, int state) {
  OdriveSerialSprintf(port, "w axis%d.requested_state %d", axis, state);
}

void setOdriveVelocity(Stream &port, int motorNumber, float velocity) {
  OdriveSerialSprintf(port, "v %d %f %f", motorNumber, velocity, 0.0f);
}

void readOdriveVariable(Stream &port, const char *varName) {
  OdriveSerialSprintf(port, "r %s", varName);
}

void writeOdriveVariable(Stream &port, const char *property, float value) {
  OdriveSerialSprintf(port, "r %s %f", property, value);
}

void writeOdriveVariable(Stream &port, const char *property, int value) {
  OdriveSerialSprintf(port, "r %s %d", property, value);
}

void writeOdriveVariable(Stream &port, const char *property, char *value) {
  OdriveSerialSprintf(port, "r %s %s", property, value);
}

int readFloat(Stream &port, double *ret, int timeout) {
  char buf[16];
  port.setTimeout(timeout);
  if (port.readBytesUntil('\n', buf, 16) <= 0)
    return -1;
  *ret = atof(buf);
  return 1;
}

int readInt(Stream &port, int *ret, int timeout) {
  char buf[16];
  port.setTimeout(timeout);
  if (port.readBytesUntil('\n', buf, 16) <= 0)
    return -1;
  *ret = atoi(buf);
  return 1;
}

int readString(Stream &port, char *ret, int timeout) {
  char buf[16];
  port.setTimeout(timeout);
  if (port.readBytesUntil('\n', buf, 16) <= 0)
    return -1;
  strcpy(ret, buf);
  return 1;
}

void OdriveSerialSprintf(Stream &port, const char *fmt, ...) {
  va_list va;
  va_start(va, fmt);
  char buf[vsnprintf(NULL, 0, fmt, va) + 1];
  vsprintf(buf, fmt, va);
  port.println(buf);
  port.flush();
  va_end(va);
}
