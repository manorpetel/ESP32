#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

constexpr uint8_t espNowChannel = 1;
// Output pins for receiver actions
const int SAFETY_OUT_PIN = 14; // GPIO14 -> safety indicator (active LOW when Safety Off)
const int LAUNCH_OUT_PIN = 27; // GPIO27 -> launch indicator (active LOW when Launch On)

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

  // Convert data to string
  String message = "";
  for (int i = 0; i < len; i++) {
    message += (char)incomingData[i];
  }

  Serial.print("Decoded string: \"");
  Serial.print(message);
  Serial.println("\"");

  // React to named messages and drive outputs accordingly
  if (message == "Safety Off") {
    digitalWrite(SAFETY_OUT_PIN, LOW); // active LOW to indicate Safety is OFF
    Serial.println("ACTION: Safety OFF -> GPIO14 LOW");
  } else if (message == "Safety On") {
    digitalWrite(SAFETY_OUT_PIN, HIGH);
    Serial.println("ACTION: Safety ON -> GPIO14 HIGH");
  } else if (message == "Launch On") {
    digitalWrite(LAUNCH_OUT_PIN, LOW); // active LOW to indicate Launch is ON
    Serial.println("ACTION: Launch ON -> GPIO27 LOW");
  } else if (message == "Launch Off") {
    digitalWrite(LAUNCH_OUT_PIN, HIGH);
    Serial.println("ACTION: Launch OFF -> GPIO27 HIGH");
  } else {
    Serial.println("NO ACTION: message not recognized");
  }

  Serial.println("========================================");
}

void setup() {
  // Configure output pins and default them HIGH (inactive)
  pinMode(SAFETY_OUT_PIN, OUTPUT);
  pinMode(LAUNCH_OUT_PIN, OUTPUT);
  digitalWrite(SAFETY_OUT_PIN, HIGH);
  digitalWrite(LAUNCH_OUT_PIN, HIGH);

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

  // Start WiFi radio (required before setting channel)
  WiFi.begin();
  delay(100);
  Serial.println("4b) WiFi radio started");

  String localMac = WiFi.macAddress();
  Serial.print("5) Receiver MAC: ");
  Serial.println(localMac);

  esp_err_t initResult = esp_now_init();
  Serial.print("6) esp_now_init() result: ");
  Serial.println(initResult);

  if (initResult != ESP_OK) {
    Serial.println("ERROR: ESP-NOW init failed");
    return;
  }

  Serial.println("DEBUG_FIX_APPLIED: About to call esp_wifi_set_channel()");
  Serial.println("DEBUG_FIX_APPLIED: Order is NOW: esp_now_init BEFORE esp_wifi_set_channel");
  
  esp_err_t chResult = esp_wifi_set_channel(espNowChannel, WIFI_SECOND_CHAN_NONE);
  Serial.print("7) esp_wifi_set_channel result: ");
  Serial.println(chResult);

  esp_err_t regResult = esp_now_register_recv_cb(OnDataRecv);
  Serial.print("8) esp_now_register_recv_cb() result: ");
  Serial.println(regResult);

  Serial.println("9) Receiver ready");
  Serial.println("Waiting for Safety/Launch ESP-NOW messages...");
}

void loop() {
  static unsigned long lastPrint = 0;

  if (millis() - lastPrint > 5000) {
    Serial.println("Receiver alive: still waiting for packet...");
    lastPrint = millis();
  }

  delay(10);
}