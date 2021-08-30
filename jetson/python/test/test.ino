void setup() {
  // 시리얼 포트 초기화
  Serial.begin(9600);
}

void loop() {
  byte buf[16];
  byte len;
  Serial.setTimeout(5);
  float axisLeft, axisRight;
  while (true) {
    // 마커 값을 기반으로 패킷을 읽음
    len = Serial.readBytesUntil(0xFF, (uint8_t *)&buf, 16);
    // buf [1] = a1x, [2] = a1y, [3] = a2x, [4] = a2y
    if (len > 0) {
      axisLeft = (buf[2] - 128) * 0.01f;
      axisRight = (buf[4] - 128) * 0.01f;
      Serial.print("axisLeft : ");
      Serial.println(axisLeft);
      Serial.print("axisRight : ");
      Serial.println(axisRight);
    }
  }
}
