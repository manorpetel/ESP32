#include <ESP32Servo.h>

double angle_num;
Servo servo_17;

void setup()
{
  	pinMode(2, OUTPUT);
	servo_17.attach(4);

	Serial.begin(115200);
	Serial.flush();
	//while(Serial.available()>0)Serial.read();

	delay(1000);
	digitalWrite(2, LOW);
	servo_17.write(0);

}


void loop()
{
	yield();

  //Serial.println(String("Enter Angle (0-180)"));
  angle_num = atof((Serial.readStringUntil('\n')).c_str());
  servo_17.write(180);
  delay(1000);
  servo_17.write(90);
  delay(1000);
  servo_17.write(0);
  delay(1000);
}