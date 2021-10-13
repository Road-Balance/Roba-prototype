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
  // float p = 1;
  // float p = 0.38f * axisLeftY;
  float TQ_FORWARD, TQ_BACKWARD;
  float FL, RL, FR, RR;
  float THR, STR, k;

  Serial.println("INIT,ENTERLOOP");
  while (true) {
    // Serial.println("LOOP");
    len = Serial.readBytesUntil(0xFF, (uint8_t *)&buf, 16);
    // buf [1, 2] = a1x, [3, 4] = a1y, [5, 6] = a2x, [7, 8] = a2y
    if (len > 0) {
      raw_a1y = buf[2] - 127; //버퍼엔 127, 152, 177.. 25단위로 들어옴
      raw_a2x = buf[3] - 127; //버퍼엔 127, 137, 147.. 10단위로 들어옴

      // -127 ~ +127
      axisLeftY = (raw_a1y)*0.02f;  // axisLeftY : 0.5, 1.0, 1.5 ...
      axisRightX = (raw_a2x)*0.01f; // axisRightX : 0.1, 0.2, 0.3 ...

      // THR : Throttle, 전후진 값, 각속도
      THR = axisLeftY;
      //최소 반경을 위한 모터 각속도 비 k
      k = -0.2f * abs(axisLeftY) + 0.6f; // ex) axisLeftY : 1.0 -> k = 0.4
      // STR : Steering, 조향값. STR은 THR보다 반드시 작음
      // 1/1.27 = 0.7874
      STR =
          THR * (1 - k) * axisRightX * 0.7874f; // axisRightX : 0.1, 0.2, 0.3...

      // Serial.print(raw_a1y);
      // Serial.print(" ");
      // Serial.println(raw_a2x);

      // Serial.print(axisLeftY);
      // Serial.print(" ");
      // Serial.println(axisRightX);

      // Serial.println();

      //전후진 && 조향, 추후 axis제어시 데드존 필요
      if (abs(THR) > 0.06f ) {
        FL = THR + STR;
        RL = THR + STR;
        FR = THR - STR;
        RR = THR - STR;
      }
      //탱크턴
      else {
        if (abs(axisRightX) <= 0.05)
          axisRightX = 0.0f;
        FL = axisRightX * 2.0f;
        RL = axisRightX * 2.0f;
        FR = -axisRightX * 2.0f;
        RR = -axisRightX * 2.0f;
      }
      TQ_FORWARD = buf[5] / 10.0;
      TQ_BACKWARD = buf[6] / 10.0;

      Serial.print(FR);
      Serial.print(" ");
      Serial.print(RR);
      Serial.print(" ");
      Serial.print(FL);
      Serial.print(" ");
      Serial.print(RL);

      Serial.print(" / ");
      Serial.print(TQ_FORWARD);
      Serial.print(" ");
      Serial.print(TQ_BACKWARD);
      Serial.print(" ");
      Serial.println();

      setOdriveVelocity(ODRIVE_FRONT_SERIAL, MOTOR_1, FR, TQ_FORWARD);
      // delay(SERIAL_DELAY);
      setOdriveVelocity(ODRIVE_FRONT_SERIAL, MOTOR_2, -FL, TQ_FORWARD);
      // delay(SERIAL_DELAY);
      setOdriveVelocity(ODRIVE_BACK_SERIAL, MOTOR_1, RR, TQ_BACKWARD);
      // delay(SERIAL_DELAY);
      setOdriveVelocity(ODRIVE_BACK_SERIAL, MOTOR_2, -RL, TQ_BACKWARD);
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

void setOdriveVelocity(Stream &port, int motorNumber, float velocity,
                       float torque_ff) {
  OdriveSerialSprintf(port, "v %d %f %f", motorNumber, velocity, torque_ff);
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
