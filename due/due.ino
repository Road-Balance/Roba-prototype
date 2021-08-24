#define ODRIVE_SERIAL Serial1

double d;
int i;

void setup() {
    Serial.begin(115200);
    Serial1.begin(115200);
    Serial.println("init");
    //Serial1.println("sr");
}

void loop() {
    readOdriveVariable("error");
    if (readInt(&i, 1) > 0) {
        if (i > 0) {
            Serial.println(i);
        } else {
            Serial.println("no err");
        }
    }
    delay(1000);
}

void setOdriveMode() {

}

void readOdriveVariable(char* varName) {
    OdriveSerialSprintf("r %s", varName);
}

void writeOdriveVariable(char* property, float value) {
    OdriveSerialSprintf("r %s %f", property, value);
}

void writeOdriveVariable(char* property, int value) {
    OdriveSerialSprintf("r %s %d", property, value);
}

void writeOdriveVariable(char* property, char* value) {
    OdriveSerialSprintf("r %s %s", property, value);
}

int readFloat(double* ret, int timeout) {
    char buf[16];
    ODRIVE_SERIAL.setTimeout(timeout * 1000);
    if (ODRIVE_SERIAL.readBytesUntil('\n', buf, 16) <= 0) return -1; 
    *ret = atof(buf);
    return 1;
}

int readInt(int* ret, int timeout) {
    char buf[16];
    ODRIVE_SERIAL.setTimeout(timeout * 1000);
    if (ODRIVE_SERIAL.readBytesUntil('\n', buf, 16) <= 0) return -1; 
    *ret = atoi(buf);
    return 1;
}

int8_t readString(char* ret, int timeout) {
    char buf[16];
    ODRIVE_SERIAL.setTimeout(timeout * 1000);
    if (ODRIVE_SERIAL.readBytesUntil('\n', buf, 16) <= 0) return -1; 
    strcpy(ret, buf);
    return 1;
}

void OdriveSerialSprintf(char *fmt, ...) {
  va_list va;
  va_start(va, fmt);
  char buf[vsnprintf(NULL, 0, fmt, va) + 1];
  vsprintf(buf, fmt, va);
  ODRIVE_SERIAL.println(buf);
  va_end(va);
}