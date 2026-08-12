#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

// Use the receiver's MAC address for a direct ESP-NOW peer.
constexpr uint8_t peerAddress[] = {0x68, 0x09, 0x47, 0x57, 0xD8, 0xBC};
constexpr uint8_t espNowChannel = 1;

void OnDataSent(const uint8_t *macAddr, esp_now_send_status_t status) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);

  Serial.print("Send status to ");
  Serial.print(macStr);
  Serial.print(": ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "success" : "FAILED");
  if (status != ESP_NOW_SEND_SUCCESS) {
    Serial.println("ERROR: Send failed!");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  WiFi.setSleep(false);
  esp_wifi_set_channel(espNowChannel, WIFI_SECOND_CHAN_NONE);

  String localMac = WiFi.macAddress();
  Serial.print("Sender MAC: ");
  Serial.println(localMac);

  char peerMacStr[18];
  snprintf(peerMacStr, sizeof(peerMacStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           peerAddress[0], peerAddress[1], peerAddress[2], peerAddress[3], peerAddress[4], peerAddress[5]);
  Serial.print("Receiver MAC: ");
  Serial.println(peerMacStr);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  Serial.println("Adding peer...");
  
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, peerAddress, 6);
  peerInfo.channel = espNowChannel;
  peerInfo.encrypt = false;

  esp_err_t addPeerResult = esp_now_add_peer(&peerInfo);
  if (addPeerResult != ESP_OK) {
    Serial.print("Failed to add peer, error code: ");
    Serial.println(addPeerResult);
    Serial.println("Attempting to continue anyway...");
  } else {
    Serial.println("Peer added successfully!");
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
        Serial.print(inputBuffer);
        Serial.print(" (length: ");
        Serial.print(inputBuffer.length());
        Serial.println(")");

        uint8_t payload[inputBuffer.length()];
        memcpy(payload, inputBuffer.c_str(), inputBuffer.length());
        
        esp_err_t result = esp_now_send(peerAddress, payload, inputBuffer.length());
        if (result != ESP_OK) {
          Serial.print("esp_now_send error: ");
          Serial.println(result);
        }
      } else if (inputBuffer.length() > 0) {
        Serial.print("Please type HIGH or LOW. You typed: ");
        Serial.println(inputBuffer);
      }

      inputBuffer = "";
    } else {
      inputBuffer += c;
    }
  }

  delay(10);
}
