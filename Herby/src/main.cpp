#include <Arduino.h>

// Last edited 12-Apr-2026 , @home, moving proj to VS code PlatformIO

// ======== runtime feature flags ========
const bool run_motors = true;
const bool run_servos = true;
const bool run_motors_esp32_output_writes = true;
const bool run_servos_esp32_output_writes = true;
const bool run_motors_debug_messages = false;
const bool run_servos_debug_messages = false;

// ======== input pins =====n
const int motors_left_right_in = 39;        // ch4 -> VN
const int motors_forward_backward_in = 34;  // ch2 -> D34
const int arm_servo_in = 35;                // D35
const int head_servo_in = 36;               // VP

// ======== output pins ========
const int motors_stby_out = 26;
const int motor_pwma_out = 23;
const int motor_ain1_out = 25;
const int motor_ain2_out = 27;
const int motor_pwmb_out = 22;
const int motor_bin1_out = 33;
const int motor_bin2_out = 32;
const int arm_servo_out = 13;
const int head_servo_out = 12;

// ======== PWM configuration ========
const int pulseTimeout = 25000; // 25 ms
const int inMin = 1118;
const int inMax = 2000;
const int outMin = 550;
const int outMax = 2600;
const int pwmFreq = 50;
const int pwmResolution = 16;
const int armServoChannel = 0;
const int headServoChannel = 1;

struct RCInputs {
  unsigned long motorsLeftRightPulse;
  unsigned long motorsForwardBackPulse;
  unsigned long armServoPulse;
  unsigned long headServoPulse;
};

struct ServoState {
  unsigned long armPulse;
  unsigned long headPulse;
  int armMapped;
  int headMapped;
  uint32_t armDuty;
  uint32_t headDuty;
};

struct MotorState {
  int stickFBspeed;
  int stickLRspeed;
  float motorLspeed;
  float motorRspeed;
  int pwmA;
  int ain1;
  int ain2;
  int pwmB;
  int bin1;
  int bin2;
  const char* motorA_mode;
  const char* motorB_mode;
};

void configurePins();
unsigned long readPulse(int pin);
RCInputs readRcInputs();
ServoState computeServoState(unsigned long armPulse, unsigned long headPulse);
void writeServoOutputs(const ServoState& state);
MotorState computeMotorState(unsigned long lrPulse, unsigned long fbPulse);
void writeMotorOutputs(const MotorState& state);
void printServoDebug(const RCInputs& inputs, const ServoState& state);
void printMotorDebug(const RCInputs& inputs, const MotorState& state);

void setup() {
  Serial.begin(115200);
  configurePins();
}

void loop() {
  RCInputs inputs = readRcInputs();
  bool printedDebug = false;

  if (run_servos) {
    ServoState servos = computeServoState(inputs.armServoPulse, inputs.headServoPulse);
    if (run_servos_esp32_output_writes) {
      writeServoOutputs(servos);
    }
    if (run_servos_debug_messages) {
      printServoDebug(inputs, servos);
      printedDebug = true;
    }
  }

  if (run_motors) {
    MotorState motors = computeMotorState(inputs.motorsLeftRightPulse, inputs.motorsForwardBackPulse);
    if (run_motors_esp32_output_writes) {
      writeMotorOutputs(motors);
    }
    if (run_motors_debug_messages) {
      printMotorDebug(inputs, motors);
      printedDebug = true;
    }
  }

  if (printedDebug) {
    delay(2500);
  }
}

void configurePins() {
  pinMode(motors_left_right_in, INPUT);
  pinMode(motors_forward_backward_in, INPUT);
  pinMode(arm_servo_in, INPUT);
  pinMode(head_servo_in, INPUT);

  pinMode(motors_stby_out, OUTPUT);
  pinMode(motor_pwma_out, OUTPUT);
  pinMode(motor_ain1_out, OUTPUT);
  pinMode(motor_ain2_out, OUTPUT);
  pinMode(motor_pwmb_out, OUTPUT);
  pinMode(motor_bin1_out, OUTPUT);
  pinMode(motor_bin2_out, OUTPUT);

  ledcSetup(armServoChannel, pwmFreq, pwmResolution);
  ledcAttachPin(arm_servo_out, armServoChannel);
  ledcSetup(headServoChannel, pwmFreq, pwmResolution);
  ledcAttachPin(head_servo_out, headServoChannel);

  if (run_motors_esp32_output_writes) {
    digitalWrite(motors_stby_out, HIGH);
  }
}

unsigned long readPulse(int pin) {
  return pulseIn(pin, HIGH, pulseTimeout);
}

RCInputs readRcInputs() {
  RCInputs inputs;
  inputs.motorsLeftRightPulse = readPulse(motors_left_right_in);
  inputs.motorsForwardBackPulse = readPulse(motors_forward_backward_in);
  inputs.armServoPulse = readPulse(arm_servo_in);
  inputs.headServoPulse = readPulse(head_servo_in);
  return inputs;
}

ServoState computeServoState(unsigned long armPulse, unsigned long headPulse) {
  ServoState state;

  state.armPulse = constrain(armPulse, inMin, inMax);
  state.headPulse = constrain(headPulse, inMin, inMax);
  state.armMapped = map(state.armPulse, inMin, inMax, outMin, outMax);
  state.headMapped = map(state.headPulse, inMin, inMax, outMin, outMax);
  state.armDuty = (uint32_t(state.armMapped) * 65535UL) / 20000UL;
  state.headDuty = (uint32_t(state.headMapped) * 65535UL) / 20000UL;

  return state;
}

void writeServoOutputs(const ServoState& state) {
  ledcWrite(armServoChannel, state.armDuty);
  ledcWrite(headServoChannel, state.headDuty);
}

MotorState computeMotorState(unsigned long lrPulse, unsigned long fbPulse) {
  const int zeroLocation = 1500;
  const int minimusSpeed = 4;
  MotorState state;

  state.stickFBspeed = int(fbPulse) - zeroLocation;
  state.stickLRspeed = int(lrPulse) - zeroLocation;
  int stickFBspeedSign = (state.stickFBspeed >= 0) ? 1 : -1;
  state.motorRspeed = constrain(state.stickFBspeed - 0.5f * state.stickLRspeed * stickFBspeedSign, -500, 500);
  state.motorLspeed = constrain(state.stickFBspeed + 0.5f * state.stickLRspeed * stickFBspeedSign, -500, 500);

  if (abs(state.motorRspeed) < minimusSpeed) {
    state.motorA_mode = "Stop";
    state.ain1 = LOW;
    state.ain2 = LOW;
    state.pwmA = 0;
  } else if (state.motorRspeed > minimusSpeed) {
    state.motorA_mode = "Forward";
    state.ain1 = HIGH;
    state.ain2 = LOW;
    state.pwmA = map(state.motorRspeed, 0, 500, 0, 255);
  } else {
    state.motorA_mode = "Backward";
    state.ain1 = LOW;
    state.ain2 = HIGH;
    state.pwmA = map(state.motorRspeed, 0, -500, 0, 255);
  }

  if (abs(state.motorLspeed) < minimusSpeed) {
    state.motorB_mode = "Stop";
    state.bin1 = LOW;
    state.bin2 = LOW;
    state.pwmB = 0;
  } else if (state.motorLspeed > minimusSpeed) {
    state.motorB_mode = "Forward";
    state.bin1 = HIGH;
    state.bin2 = LOW;
    state.pwmB = map(state.motorLspeed, 0, 500, 0, 255);
  } else {
    state.motorB_mode = "Backward";
    state.bin1 = LOW;
    state.bin2 = HIGH;
    state.pwmB = map(state.motorLspeed, 0, -500, 0, 255);
  }

  return state;
}

void writeMotorOutputs(const MotorState& state) {
  digitalWrite(motor_ain1_out, state.ain1);
  digitalWrite(motor_ain2_out, state.ain2);
  analogWrite(motor_pwma_out, state.pwmA);
  digitalWrite(motor_bin1_out, state.bin1);
  digitalWrite(motor_bin2_out, state.bin2);
  analogWrite(motor_pwmb_out, state.pwmB);
}

void printServoDebug(const RCInputs& inputs, const ServoState& state) {
  Serial.print("Servo pulses: LR=");
  Serial.print(inputs.motorsLeftRightPulse);
  Serial.print(" FB=");
  Serial.print(inputs.motorsForwardBackPulse);
  Serial.print(" | ARM=");
  Serial.print(state.armPulse);
  Serial.print(" -> ");
  Serial.print(state.armMapped);
  Serial.print(" | HEAD=");
  Serial.print(state.headPulse);
  Serial.print(" -> ");
  Serial.println(state.headMapped);
}

void printMotorDebug(const RCInputs& inputs, const MotorState& state) {
  Serial.println();
  Serial.print("Motor pulses: FB=");
  Serial.print(inputs.motorsForwardBackPulse);
  Serial.print(" LR=");
  Serial.println(inputs.motorsLeftRightPulse);
  Serial.print("Stick speeds: FB=");
  Serial.print(state.stickFBspeed);
  Serial.print(" LR=");
  Serial.println(state.stickLRspeed);
  Serial.print("Motor speeds: L=");
  Serial.print(state.motorLspeed);
  Serial.print(" R=");
  Serial.println(state.motorRspeed);
  Serial.print("Motor A: ");
  Serial.print(state.motorA_mode);
  Serial.print(" PWM=");
  Serial.println(state.pwmA);
  Serial.print("Motor B: ");
  Serial.print(state.motorB_mode);
  Serial.print(" PWM=");
  Serial.println(state.pwmB);
  Serial.println();
  Serial.println("------------------------------------------------------------");
}
