#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

// Sender project: upload this firmware to the board that should send HIGH/LOW.
// It targets the receiver board MAC below.
constexpr uint8_t peerAddress[] = {0xFC, 0xE8, 0xC0, 0xE0, 0xC7, 0xA4};

void OnDataSent(const uint8_t *macAddr, esp_now_send_status_t status) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);

  Serial.print("Send status to ");
  Serial.print(macStr);
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? ": success" : ": failed");
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, peerAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  Serial.println("Sender ready");
  Serial.println("Type HIGH or LOW and press Enter.");
}

void loop() {
  static String inputBuffer = "";

  while (Serial.available() > 0) {
    char c = Serial.read();

    if (c == '\r' || c == '\n') {
      inputBuffer.trim();

      if (inputBuffer == "HIGH" || inputBuffer == "LOW") {
        Serial.print("Sending: ");
        Serial.println(inputBuffer);

        uint8_t payload[inputBuffer.length() + 1];
        strcpy((char *)payload, inputBuffer.c_str());
        esp_now_send(peerAddress, payload, inputBuffer.length() + 1);
      } else {
        Serial.println("Please type HIGH or LOW.");
      }

      inputBuffer = "";
    } else {
      inputBuffer += c;
    }
  }

  delay(10);
}
