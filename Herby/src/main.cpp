#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  delay(1000);  // IMPORTANT for ESP32

  Serial.println("=== STARTED ===");
}

void loop() {
  Serial.println("Running...");
  delay(1000);
}