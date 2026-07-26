#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

// Receiver project: upload this firmware to the board that should react to HIGH/LOW.
// It expects messages from the sender board MAC below.
constexpr uint8_t peerAddress[] = {0xFC, 0xE8, 0xC0, 0xE1, 0x85, 0xE0};

void OnDataRecv(const uint8_t *macAddr, const uint8_t *incomingData, int len) {
  String message = "";
  for (int i = 0; i < len; i++) {
    message += (char)incomingData[i];
  }

  Serial.print("Received: ");
  Serial.println(message);

  if (message == "HIGH") {
    digitalWrite(LED_BUILTIN, HIGH);
  } else if (message == "LOW") {
    digitalWrite(LED_BUILTIN, LOW);
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, peerAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  Serial.println("Receiver ready");
  Serial.println("Waiting for HIGH/LOW commands.");
}

void loop() {
  delay(10);
}
