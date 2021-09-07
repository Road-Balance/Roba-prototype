#define ODRIVE_FRONT_SERIAL Serial1
#define ODRIVE_BACK_SERIAL Serial2
#define ODRIVE_AXIS_STATE_CLOSED_LOOP_CONTROL 8

#define ODRIVE_COUNT 2
#define ODRIVE_AXIS_PER_BOARD 2
#define MOTOR_1 0
#define MOTOR_2 1

int axises[2] = {MOTOR_1, MOTOR_2};

void setup()
{
    ODRIVE_FRONT_SERIAL.begin(115200);
    ODRIVE_BACK_SERIAL.begin(115200);
    Serial.begin(115200);
    Serial.println("INIT,UARTOK");

    OdriveSerialSprintf(ODRIVE_FRONT_SERIAL, "sr");
    OdriveSerialSprintf(ODRIVE_BACK_SERIAL, "sr");
    delay(500);
    Serial.println("INIT,REBOOTOK");

    setOdriveClosedLoop(ODRIVE_FRONT_SERIAL);
    setOdriveClosedLoop(ODRIVE_BACK_SERIAL);
    Serial.println("INIT,SETSTATEOK");
}

void loop()
{
    byte buf[16];
    byte len;
    Serial.setTimeout(50);
    float axisLeft, axisRight;
    int i;

    Serial.println("INIT,ENTERLOOP");
    while (true)
    {
        //Serial.println("LOOP");
        len = Serial.readBytesUntil(0xFF, (uint8_t *)&buf, 16);
        // buf [1] = a1x, [2] = a1y, [3] = a2x, [4] = a2y
        if (len > 0)
        {
            axisLeft = (buf[2] - 128) * 0.01f;
            axisRight = (buf[3] - 128) * 0.01f;
            setOdriveVelocity(ODRIVE_FRONT_SERIAL, MOTOR_1, -axisLeft);
            setOdriveVelocity(ODRIVE_FRONT_SERIAL, MOTOR_2, axisRight);
            setOdriveVelocity(ODRIVE_BACK_SERIAL, MOTOR_1, -axisLeft);
            setOdriveVelocity(ODRIVE_BACK_SERIAL, MOTOR_2, axisRight);
        }

        readOdriveVariable(ODRIVE_FRONT_SERIAL, "error");
        if (readInt(ODRIVE_FRONT_SERIAL, &i, 50) > 0)
        {
            if (i != 0)
            {
                Serial.println("ERR,FRONT");
                ODRIVE_FRONT_SERIAL.println("sc");
                delay(50);
                setOdriveClosedLoop(ODRIVE_FRONT_SERIAL);
            }
        }
        delay(50);
        readOdriveVariable(ODRIVE_FRONT_BACK, "error");
        if (readInt(ODRIVE_BACK_SERIAL, &i, 50) > 0)
        {
            if (i != 0)
            {
                Serial.println("ERR,BACK");
                ODRIVE_BACK_SERIAL.println("sc");
                delay(50);
                setOdriveClosedLoop(ODRIVE_BACK_SERIAL);
            }
        }
    }
}

void setOdriveClosedLoop(Stream &port)
{
    for (int a = 0; a < ODRIVE_AXIS_PER_BOARD; a++)
    {
        setOdriveSetState(port, axises[a], ODRIVE_AXIS_STATE_CLOSED_LOOP_CONTROL);
    }
}

void setOdriveSetState(Stream &port, int axis, int state)
{
    OdriveSerialSprintf(port, "w axis%d.requested_state %d", axis, state);
}

void setOdriveVelocity(Stream &port, int motorNumber, float velocity)
{
    OdriveSerialSprintf(port, "v %d %f %f", motorNumber, velocity, 0.0f);
    delay(10);
}

void readOdriveVariable(Stream &port, const char *varName)
{
    OdriveSerialSprintf(port, "r %s", varName);
}

void writeOdriveVariable(Stream &port, const char *property, float value)
{
    OdriveSerialSprintf(port, "r %s %f", property, value);
}

void writeOdriveVariable(Stream &port, const char *property, int value)
{
    OdriveSerialSprintf(port, "r %s %d", property, value);
}

void writeOdriveVariable(Stream &port, const char *property, char *value)
{
    OdriveSerialSprintf(port, "r %s %s", property, value);
}

int readFloat(Stream &port, double *ret, int timeout)
{
    char buf[16];
    port.setTimeout(timeout);
    if (port.readBytesUntil('\n', buf, 16) <= 0)
        return -1;
    *ret = atof(buf);
    return 1;
}

int readInt(Stream &port, int *ret, int timeout)
{
    char buf[16];
    port.setTimeout(timeout);
    if (port.readBytesUntil('\n', buf, 16) <= 0)
        return -1;
    *ret = atoi(buf);
    return 1;
}

int readString(Stream &port, char *ret, int timeout)
{
    char buf[16];
    port.setTimeout(timeout);
    if (port.readBytesUntil('\n', buf, 16) <= 0)
        return -1;
    strcpy(ret, buf);
    return 1;
}

void OdriveSerialSprintf(Stream &port, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    char buf[vsnprintf(NULL, 0, fmt, va) + 1];
    vsprintf(buf, fmt, va);
    port.println(buf);
    va_end(va);
}