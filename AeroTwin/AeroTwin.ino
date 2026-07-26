#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>

const char* WIFI_SSID = "WiFI NAME";
const char* WIFI_PASS = "WIFI PASSWORD";

const int MPU_ADDR = 0x68;
int16_t accX, accY, accZ, gyroX, gyroY, gyroZ;

float roll = 0, pitch = 0;
unsigned long lastTime = 0;

WebServer server(80);
char json[96];

void readMPU() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true);

  accX = Wire.read() << 8 | Wire.read();
  accY = Wire.read() << 8 | Wire.read();
  accZ = Wire.read() << 8 | Wire.read();
  Wire.read(); Wire.read();
  gyroX = Wire.read() << 8 | Wire.read();
  gyroY = Wire.read() << 8 | Wire.read();
  gyroZ = Wire.read() << 8 | Wire.read();
}

void updateOrientation() {
  readMPU();

  unsigned long now = millis();
  float dt = (now - lastTime) / 1000.0;
  lastTime = now;

  float accAngleX = atan2(accY, accZ) * 180.0 / PI;
  float accAngleY = atan2(-accX, sqrt((float)accY * accY + (float)accZ * accZ)) * 180.0 / PI;

  float gyroRateX = gyroX / 131.0;
  float gyroRateY = gyroY / 131.0;

  roll  = 0.98 * (roll  + gyroRateX * dt) + 0.02 * accAngleX;
  pitch = 0.98 * (pitch + gyroRateY * dt) + 0.02 * accAngleY;
}

void handleData() {
  snprintf(json, sizeof(json), "{\"roll\":%.2f,\"pitch\":%.2f,\"t\":%lu}", roll, pitch, millis());
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
}

void handleRoot() {
  server.send(200, "text/plain", "AeroTwin ESP32 is running. Visit /data for JSON.");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin(21, 22);

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();

  lastTime = millis();
  Serial.println("AeroTwin HTTP server ready - streaming MPU6050 data via /data");
}

void loop() {
  server.handleClient();

  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate >= 15) {  // steady ~66Hz sensor fusion, independent of network timing
    lastUpdate = millis();
    updateOrientation();
  }
}
