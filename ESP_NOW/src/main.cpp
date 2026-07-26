#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

// ESP32 MAC addresses:
// Board 1 MAC: FC:E8:C0:E1:85:E0
// Board 2 MAC: FC:E8:C0:E0:C7:A4
// Set BOARD_MODE to 1 for sender mode and 2 for receiver mode.
constexpr int BOARD_MODE = 1;  // change to 2 on the other board

const uint8_t peerAddress[] = {0xFC, 0xE8, 0xC0, 0xE0, 0xC7, 0xA4};  // change to the other board's MAC on the other board

void OnDataSent(const uint8_t *macAddr, esp_now_send_status_t status) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);

  Serial.print("Send status to ");
  Serial.print(macStr);
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? ": success" : ": failed");
}

void OnDataRecv(const uint8_t *macAddr, const uint8_t *incomingData, int len) {
  if (BOARD_MODE != 2) {
    return;
  }

  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);

  String message = "";
  for (int i = 0; i < len; i++) {
    message += (char)incomingData[i];
  }

  Serial.print("Received from ");
  Serial.print(macStr);
  Serial.print(": ");
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

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, peerAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  Serial.println("ESP-NOW ready");
  Serial.print("My MAC: ");
  Serial.println(WiFi.macAddress());

  if (BOARD_MODE == 1) {
    Serial.println("Sender mode: type HIGH or LOW and press Enter.");
  } else {
    Serial.println("Receiver mode: waiting for HIGH/LOW commands.");
  }
}

void loop() {
  if (BOARD_MODE == 1) {
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
  }

  delay(10);
}