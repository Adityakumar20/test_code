#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>

// WiFi credentials
const char* ssid = "test";
const char* password = "12345678";

// OTA firmware location
const char* firmwareUrl = "https://raw.githubusercontent.com/Adityakumar20/test_code/main/test/build/esp32.esp32.esp32/test.ino.bin";

// Firmware version (manually increment when you build new)
const char* firmwareVersion = "v1.0.1";

unsigned long lastBlink = 0;
bool ledState = false;

void setup() {
  pinMode(2, OUTPUT);
  Serial.begin(115200);
  delay(100);

  Serial.println("⏱️ ESP32 Booting...");
  Serial.println("📦 Current firmware version: " + String(firmwareVersion));

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("🌐 Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected");

  // Begin OTA Update
  Serial.println("🛰️ Starting OTA update check...");
  HTTPClient http;
  http.begin(firmwareUrl);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    int contentLength = http.getSize();
    if (contentLength <= 0) {
      Serial.println("⚠️ Content-Length not available.");
      return;
    }

    bool canBegin = Update.begin(contentLength);
    if (!canBegin) {
      Serial.println("❌ Not enough space to begin OTA.");
      return;
    }

    WiFiClient* client = http.getStreamPtr();
    size_t written = Update.writeStream(*client);

    if (written == contentLength) {
      Serial.println("✅ OTA written successfully.");
    } else {
      Serial.println("❌ OTA write failed.");
    }

    if (Update.end()) {
      if (Update.isFinished()) {
        Serial.println("🚀 OTA update complete. Restarting...");
        ESP.restart();
      } else {
        Serial.println("⚠️ OTA not fully completed.");
      }
    } else {
      Serial.printf("❌ OTA error: %d\n", Update.getError());
    }

  } else {
    Serial.printf("❌ OTA firmware download failed. HTTP code: %d\n", httpCode);
  }

  http.end();
  Serial.println("ℹ️ No OTA update or already latest firmware.");
}

void loop() {
  // Blink LED
  if (millis() - lastBlink > 500) {
    ledState = !ledState;
    digitalWrite(2, ledState);
    Serial.println(String(firmwareVersion));
    lastBlink = millis();
  }
}
