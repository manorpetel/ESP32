#include <Arduino.h>

const int servo_arm_inputPin  = 12;
const int servo_arm_outputPin = 32;

const int servo_head_inputPin  = 35;
const int servo_head_outputPin = 34;

// Input pulse measurement
volatile unsigned long servo_arm_riseTime = 0;
volatile unsigned long servo_arm_inputPulse = 1500;

volatile unsigned long servo_head_riseTime = 0;
volatile unsigned long servo_head_inputPulse = 1500;

// Mapping limits
const int inMin  = 1118;
const int inMax  = 2000;
const int outMin = 550;
const int outMax = 2600;

// PWM settings
const int pwmFreq = 50;
const int pwmResolution = 16;

const int servo_arm_pwmChannel = 0;
const int servo_head_pwmChannel = 1;


// ---------- INTERRUPTS ----------

void IRAM_ATTR servo_arm_isr() {
  if (digitalRead(servo_arm_inputPin) == HIGH) {
    servo_arm_riseTime = micros();
  } else {
    servo_arm_inputPulse = micros() - servo_arm_riseTime;
  }
}

void IRAM_ATTR servo_head_isr() {
  if (digitalRead(servo_head_inputPin) == HIGH) {
    servo_head_riseTime = micros();
  } else {
    servo_head_inputPulse = micros() - servo_head_riseTime;
  }
}


// ---------- SETUP ----------

void setup() {

  Serial.begin(115200);

  pinMode(servo_arm_inputPin, INPUT);
  pinMode(servo_head_inputPin, INPUT);

  attachInterrupt(digitalPinToInterrupt(servo_arm_inputPin), servo_arm_isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(servo_head_inputPin), servo_head_isr, CHANGE);

  // Servo ARM PWM
  ledcSetup(servo_arm_pwmChannel, pwmFreq, pwmResolution);
  ledcAttachPin(servo_arm_outputPin, servo_arm_pwmChannel);

  // Servo HEAD PWM
  ledcSetup(servo_head_pwmChannel, pwmFreq, pwmResolution);
  ledcAttachPin(servo_head_outputPin, servo_head_pwmChannel);
}


// ---------- LOOP ----------

void loop() {

  // Copy volatile safely
  unsigned long servo_arm_pulse;
  unsigned long servo_head_pulse;

  noInterrupts();
  servo_arm_pulse = servo_arm_inputPulse;
  servo_head_pulse = servo_head_inputPulse;
  interrupts();

  // Constrain inputs
  servo_arm_pulse  = constrain(servo_arm_pulse, inMin, inMax);
  servo_head_pulse = constrain(servo_head_pulse, inMin, inMax);

  // Map ranges
  long servo_arm_mapped  = map(servo_arm_pulse,  inMin, inMax, outMin, outMax);
  long servo_head_mapped = map(servo_head_pulse, inMin, inMax, outMin, outMax);

  // Convert microseconds to duty cycle
  uint32_t servo_arm_duty  = (servo_arm_mapped  * 65535) / 20000;
  uint32_t servo_head_duty = (servo_head_mapped * 65535) / 20000;

  ledcWrite(servo_arm_pwmChannel, servo_arm_duty);
  ledcWrite(servo_head_pwmChannel, servo_head_duty);

  Serial.print("ARM In:");
  Serial.print(servo_arm_pulse);
  Serial.print("us Out:");
  Serial.print(servo_arm_mapped);

  Serial.print(" | HEAD In:");
  Serial.print(servo_head_pulse);
  Serial.print("us Out:");
  Serial.println(servo_head_mapped);

  delay(20);
}