#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>

const char* ssid = "test";
const char* password = "12345678";

const char* firmwareUrl = "https://raw.githubusercontent.com/Adityakumar20/test_code/main/test/build/esp32.esp32.esp32/test.ino.bin";
const char* versionUrl = "https://raw.githubusercontent.com/Adityakumar20/test_code/main/version.txt";

const char* currentVersion = "v1.0.2";

unsigned long lastBlink = 0;
bool ledState = false;

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

  // 🔍 Step 1: Check version file
  HTTPClient http;
  http.begin(versionUrl);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String newVersion = http.getString();
    newVersion.trim();

    Serial.println("📥 Online firmware version: " + newVersion);

    if (newVersion != currentVersion) {
      Serial.println("⬆️ Update available! Starting OTA...");

      // 🔄 Download and apply firmware
      http.end();
      http.begin(firmwareUrl);
      int fwCode = http.GET();

      if (fwCode == HTTP_CODE_OK) {
        int contentLength = http.getSize();
        if (Update.begin(contentLength)) {
          WiFiClient* stream = http.getStreamPtr();
          size_t written = Update.writeStream(*stream);
          if (written == contentLength) {
            Serial.println("✅ OTA written successfully.");
          } else {
            Serial.println("❌ OTA write incomplete.");
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
          Serial.println("❌ Not enough space for OTA");
        }

      } else {
        Serial.println("❌ Firmware download failed. HTTP code: " + String(fwCode));
      }

    } else {
      Serial.println("✅ Firmware is up-to-date. No OTA needed.");
    }

  } else {
    Serial.println("❌ Version check failed. HTTP code: " + String(httpCode));
  }

  http.end();
}

void loop() {
  if (millis() - lastBlink > 300) {
    ledState = !ledState;
    digitalWrite(2, ledState);
    Serial.println(String(currentVersion));
    lastBlink = millis();
  }
}
