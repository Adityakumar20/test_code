#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>

const char* ssid = "test";  // 🔁 Change this
const char* password = "12345678";  // 🔁 Change this

const char* firmwareUrl = "https://raw.githubusercontent.com/Adityakumar20/test_code/main/test/build/esp32.esp32.esp32/test.ino.bin";

unsigned long lastBlink = 0;
bool ledState = false;

void setup() {
  pinMode(2, OUTPUT);  // On-board LED
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");

  // Start OTA update
  HTTPClient http;
  http.begin(firmwareUrl);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    int contentLength = http.getSize();
    bool canBegin = Update.begin(contentLength);

    if (canBegin) {
      WiFiClient* client = http.getStreamPtr();
      size_t written = Update.writeStream(*client);
      if (written == contentLength) {
        Serial.println("OTA written successfully.");
      } else {
        Serial.println("OTA write failed.");
      }

      if (Update.end()) {
        Serial.println("OTA update complete.");
        if (Update.isFinished()) {
          Serial.println("Restarting ESP32...");
          ESP.restart();
        } else {
          Serial.println("OTA not finished.");
        }
      } else {
        Serial.println("OTA Error: " + String(Update.getError()));
      }

    } else {
      Serial.println("Not enough space for OTA");
    }

  } else {
    Serial.println("Firmware download failed. HTTP code: " + String(httpCode));
  }

  http.end();
}

void loop() {
  // Blink LED
  if (millis() - lastBlink > 1000) {
    ledState = !ledState;
    digitalWrite(2, ledState);
    Serial.println("Version 0.1");
    lastBlink = millis();
  }
}
