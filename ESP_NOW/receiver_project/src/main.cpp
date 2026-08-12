#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

constexpr uint8_t espNowChannel = 1;

void printMac(const uint8_t *mac) {
  char buf[18];
  snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.println(buf);
}

void OnDataRecv(const uint8_t *macAddr, const uint8_t *incomingData, int len) {
  Serial.println("========================================");
  Serial.println("CALLBACK: OnDataRecv() triggered");
  Serial.print("Received packet length: ");
  Serial.println(len);

  Serial.print("Source MAC: ");
  printMac(macAddr);

  Serial.print("Raw bytes: ");
  for (int i = 0; i < len; i++) {
    Serial.print((char)incomingData[i]);
    Serial.print(" ");
  }
  Serial.println();

  // Convert data to string
  String message = "";
  for (int i = 0; i < len; i++) {
    message += (char)incomingData[i];
  }

  Serial.print("Decoded string: \"");
  Serial.print(message);
  Serial.println("\"");

  if (message == "HIGH") {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("MATCH: message == HIGH");
  } else if (message == "LOW") {
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("MATCH: message == LOW");
  } else {
    Serial.println("NO MATCH: message is not HIGH or LOW");
  }

  Serial.println("========================================");
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.begin(115200);
  delay(1000);

  Serial.println("=== RECEIVER DEBUG START ===");

  Serial.print("1) WiFi.mode() before: ");
  Serial.println(WiFi.getMode());

  WiFi.mode(WIFI_STA);
  Serial.print("2) WiFi.mode() after WiFi.mode(WIFI_STA): ");
  Serial.println(WiFi.getMode());

  WiFi.disconnect(true);
  Serial.println("3) WiFi disconnected");

  WiFi.setSleep(false);
  Serial.println("4) WiFi sleep disabled");

  esp_err_t chResult = esp_wifi_set_channel(espNowChannel, WIFI_SECOND_CHAN_NONE);
  Serial.print("5) esp_wifi_set_channel result: ");
  Serial.println(chResult);

  String localMac = WiFi.macAddress();
  Serial.print("6) Receiver MAC: ");
  Serial.println(localMac);

  esp_err_t initResult = esp_now_init();
  Serial.print("7) esp_now_init() result: ");
  Serial.println(initResult);

  if (initResult != ESP_OK) {
    Serial.println("ERROR: ESP-NOW init failed");
    return;
  }

  esp_err_t regResult = esp_now_register_recv_cb(OnDataRecv);
  Serial.print("8) esp_now_register_recv_cb() result: ");
  Serial.println(regResult);

  Serial.println("9) Receiver ready");
  Serial.println("Waiting for HIGH/LOW ESP-NOW messages...");
}

void loop() {
  static unsigned long lastPrint = 0;

  if (millis() - lastPrint > 5000) {
    Serial.println("Receiver alive: still waiting for packet...");
    lastPrint = millis();
  }

  delay(10);
}