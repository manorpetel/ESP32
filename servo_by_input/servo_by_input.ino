#include <ESP32Servo.h>

int angle;
Servo servo_17;

void setup() {
	servo_17.attach(4);
	servo_17.write(0);
  Serial.begin(115200);
	Serial.println("Enter servo angle (0-180)");
}

void loop() {
  
  if (Serial.available() > 0) {
    angle = Serial.parseInt();
    Serial.print("The angle you entered is : ");
    Serial.println(angle);
		servo_17.write(angle);
    delay(2000);
		Serial.println("Enter servo angle (0-180)");
  }
}