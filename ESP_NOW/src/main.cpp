#include <Arduino.h>
//edited @ home , 19 Jul 2026 . initial ESP now 

// put function declarations here:
int myFunction(int, int);

void setup() {
  Serial.begin(115200);
}

void loop() {
  Serial.println("Hi Alon");
  delay(3000);
  Serial.println("Hi Manor");
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}