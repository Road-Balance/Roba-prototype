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
  delay(2000);
}

void loop() {
  byte buf[16];
  byte len;
  Serial.setTimeout(5);
  float axisLeftY, axisRightY, axisRightX;
  float offsetLeft, offsetRight;
  int raw_a1y, raw_a2x;
  double vbusf, vbusb;

  // 추후 axisLeftY +- p*axisRightX의 값이 axisLeftY의 부호를 그대로 따라가야
  // 함. 1/2.55 = 0.38...
  float p = 1;
  float p = 0.38f * axisLeftY;
  float FL, RL, FR, RR;

  Serial.println("INIT,ENTERLOOP");
  while (true) {
    // Serial.println("LOOP");
    len = Serial.readBytesUntil(0xFF, (uint8_t *)&buf, 16);
    // buf [1, 2] = a1x, [3, 4] = a1y, [5, 6] = a2x, [7, 8] = a2y
    if (len > 0) {
      raw_a1y = buf[2] - 128;
      raw_a2x = buf[3] - 128;
      axisLeftY = (raw_a1y)*0.02f;
      axisRightX = (raw_a2x)*0.02f;
      //사용안함 axisRightY = ((buf[7] * 255 + buf[8]) - 32767) * 0.0001f;
      Serial.print(raw_a1y);
      Serial.print(" ");
      Serial.println(raw_a2x);

      Serial.print(axisLeftY);
      Serial.print(" ");
      Serial.println(axisRightX);

      Serial.println();

      if (axisLeftY > 0.02f) //전진
      {
        Serial.println("frontOK");
        //여기서 axisLeftY뒤에 세미콜론이 찍혀있었다..
        FR = axisLeftY - p * axisRightX; //우측 전륜
        RR = axisLeftY - p * axisRightX; //우측 후륜
        FL = axisLeftY + p * axisRightX; //좌측 전륜
        RL = axisLeftY + p * axisRightX; //좌측 후륜
      }

      else if (axisLeftY < -0.02f) //후진
      {
        Serial.println("RearOK");
        FR = axisLeftY - p * axisRightX;
        RR = axisLeftY - p * axisRightX;
        FL = axisLeftY + p * axisRightX;
        RL = axisLeftY + p * axisRightX;
      }

      else //탱크턴
      {
        FR = axisRightX;
        RR = axisRightX;
        FL = -axisRightX;
        RL = -axisRightX;
      }

      Serial.print(FR);
      Serial.print(" ");
      Serial.print(RR);
      Serial.print(" ");
      Serial.print(FL);
      Serial.print(" ");
      Serial.print(RL);
      Serial.print(" ");
      Serial.println();

      setOdriveVelocity(ODRIVE_FRONT_SERIAL, MOTOR_1, FR);
      // delay(SERIAL_DELAY);
      setOdriveVelocity(ODRIVE_FRONT_SERIAL, MOTOR_2, -FL);
      // delay(SERIAL_DELAY);
      setOdriveVelocity(ODRIVE_BACK_SERIAL, MOTOR_1, RR);
      // delay(SERIAL_DELAY);
      setOdriveVelocity(ODRIVE_BACK_SERIAL, MOTOR_2, -RL);
      // delay(SERIAL_DELAY);
    }
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
