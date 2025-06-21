#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>

const char* ssid = "test";
const char* password = "12345678";

const char* firmwareUrl = "https://raw.githubusercontent.com/Adityakumar20/test_code/main/test/build/esp32.esp32.esp32/test.ino.bin";
const char* versionUrl = "https://raw.githubusercontent.com/Adityakumar20/test_code/main/version.txt";
const char* currentVersion = "v1.0.7";  // Update this in code when you release new firmware

unsigned long lastBlink = 0;
bool ledState = false;

unsigned long lastOTACheck = 0;
const unsigned long otaInterval = 30000;  // 30 seconds

void setup() {
  pinMode(2, OUTPUT);
  Serial.begin(115200);
  delay(100);

  Serial.println("⏱️ ESP32 Booting...");
  Serial.println("📦 Current firmware version: " + String(currentVersion));

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("🌐 Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected");
}

void loop() {
  // LED Blink
  if (millis() - lastBlink > 1000) {
    ledState = !ledState;
    digitalWrite(2, ledState);
    lastBlink = millis();
  }

  // OTA version check every 30 sec
  if (millis() - lastOTACheck > otaInterval) {
    lastOTACheck = millis();
    checkForUpdate();
  }
}

void checkForUpdate() {
  Serial.println("\n🔍 Checking for OTA update...");

  HTTPClient http;
  http.begin(versionUrl);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String newVersion = http.getString();
    newVersion.trim();
    Serial.println("📥 Online firmware version: " + newVersion);

    if (newVersion != currentVersion) {
      Serial.println("⬆️ New version found! Starting OTA update...");
      http.end();
      doOTAUpdate();
    } else {
      Serial.println("✅ Already on latest version.");
    }
  } else {
    Serial.println("❌ Failed to fetch version.txt. Code: " + String(httpCode));
  }

  http.end();
}

void doOTAUpdate() {
  HTTPClient http;
  http.begin(firmwareUrl);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    int contentLength = http.getSize();
    if (Update.begin(contentLength)) {
      WiFiClient* stream = http.getStreamPtr();
      size_t written = Update.writeStream(*stream);

      if (written == contentLength) {
        Serial.println("✅ OTA written successfully.");
      } else {
        Serial.println("❌ Incomplete OTA write.");
      }

      if (Update.end()) {
        if (Update.isFinished()) {
          Serial.println("🚀 OTA complete. Restarting...");
          ESP.restart();
        } else {
          Serial.println("⚠️ OTA not finished properly.");
        }
      } else {
        Serial.println("❌ OTA error: " + String(Update.getError()));
      }
    } else {
      Serial.println("❌ Not enough space to OTA.");
    }
  } else {
    Serial.println("❌ Failed to download firmware. Code: " + String(httpCode));
  }

  http.end();
}
