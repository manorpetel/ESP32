#include <Arduino.h>

// Last edited 12-Jul-2026 , @rLab, LaunchPad init. 
// put function declarations here:
int myFunction(int, int);

const int relay1_out = 12;
const int relay2_out = 14;

void setup() {
  //Serial.begin(115200);
  // ======== output pins ========
  pinMode(relay1_out, OUTPUT);
  pinMode(relay2_out, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(relay1_out, LOW);
  digitalWrite(relay2_out, LOW);
  delay(1000);
  digitalWrite(relay1_out, HIGH);
  digitalWrite(relay2_out, HIGH);
  delay(5000);
  digitalWrite(relay1_out, LOW);
  digitalWrite(relay2_out, LOW);
  delay(1000);
  digitalWrite(relay1_out, HIGH);
  digitalWrite(relay2_out, HIGH);
  delay(5000);
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}