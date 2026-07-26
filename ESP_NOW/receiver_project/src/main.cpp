#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

constexpr uint8_t espNowChannel = 1;

void OnDataRecv(const uint8_t *macAddr, const uint8_t *incomingData, int len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);

  String message = String((const char *)incomingData, len);
  message.trim();

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
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  WiFi.setSleep(false);
  esp_wifi_set_channel(espNowChannel, WIFI_SECOND_CHAN_NONE);

  Serial.print("Receiver MAC: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("Receiver ready");
  Serial.println("Waiting for HIGH/LOW commands.");
}

void loop() {
  delay(10);
}
