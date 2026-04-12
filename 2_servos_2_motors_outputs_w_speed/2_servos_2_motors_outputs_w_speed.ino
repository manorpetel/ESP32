// Last edited 06-Apr-2026 , @rlab


const int motors_left_right_in = 39;        // ch4 -> VN
const int motors_forward_backward_in = 34;  // ch2 -> D34
const int motors_stby_out = 26;
const int motor_pwma_out = 23; 
const int motor_ain1_out = 25;
const int motor_ain2_out = 27;
const int motor_pwmb_out = 22; 
const int motor_bin1_out = 33;
const int motor_bin2_out = 32;

const int arm_servo_in = 35;                // D35
const int head_servo_in = 36;               // VP
const int arm_servo_out  = 13;              // D13
const int head_servo_out = 12;              // D12


const int pulseTimeout = 25000; // 25ms

// mapping limits (same as your previous code)
const int inMin  = 1118;
const int inMax  = 2000;
const int outMin = 550;
const int outMax = 2600;

// PWM
const int pwmFreq = 50;
const int pwmResolution = 16;

void setup() {

  Serial.begin(115200);

  pinMode(motors_left_right_in, INPUT);
  pinMode(motors_forward_backward_in, INPUT);
  pinMode(arm_servo_in, INPUT);
  pinMode(head_servo_in, INPUT);

  ledcAttach(arm_servo_out, pwmFreq, pwmResolution);
  ledcAttach(head_servo_out, pwmFreq, pwmResolution);
  //ledcAttach(motor_pwma_out, pwmFreq, pwmResolution);

  pinMode(motors_stby_out, OUTPUT);
  pinMode(motor_pwma_out, OUTPUT);
  pinMode(motor_ain1_out, OUTPUT);
  pinMode(motor_ain2_out, OUTPUT);
  pinMode(motor_pwmb_out, OUTPUT);
  pinMode(motor_bin1_out, OUTPUT);
  pinMode(motor_bin2_out, OUTPUT);
}

void loop() {

  // ---- read inputs ----

  unsigned long motors_lr_pulse =
      pulseIn(motors_left_right_in, HIGH, pulseTimeout);

  unsigned long motors_fb_pulse =
      pulseIn(motors_forward_backward_in, HIGH, pulseTimeout);

  unsigned long arm_pulse =
      pulseIn(arm_servo_in, HIGH, pulseTimeout);

  unsigned long head_pulse =
      pulseIn(head_servo_in, HIGH, pulseTimeout);

  // ---- normalize servo inputs ----

  arm_pulse  = constrain(arm_pulse, inMin, inMax);
  head_pulse = constrain(head_pulse, inMin, inMax);

  long arm_mapped  = map(arm_pulse,  inMin, inMax, outMin, outMax);
  long head_mapped = map(head_pulse, inMin, inMax, outMin, outMax);

  // convert to duty cycle
  uint32_t arm_duty  = (arm_mapped  * 65535) / 20000;
  uint32_t head_duty = (head_mapped * 65535) / 20000;

  // ---- servo outputs ----

  ledcWrite(arm_servo_out, arm_duty);
  ledcWrite(head_servo_out, head_duty);
  //ledcWrite(motor_pwma_out, arm_duty);

  // ---- debug output ----
  int debug_servos;
  debug_servos = 1;
  if (debug_servos) {
  Serial.print("LR:");
  Serial.print(motors_lr_pulse);

  Serial.print(" | FB:");
  Serial.print(motors_fb_pulse);

  Serial.print(" | ARM:");
  Serial.print(arm_pulse);
  Serial.print("->");
  Serial.print(arm_mapped);

  Serial.print(" | HEAD:");
  Serial.print(head_pulse);
  Serial.print("->");
  Serial.print(head_mapped);
  Serial.println();
  delay(1000);
}

  setMotorA(motors_lr_pulse, motors_fb_pulse);
}

void setMotorA (unsigned long motors_lr_pulse , unsigned long motors_fb_pulse) {
  const int zero_location = 1500 ; 
  const int zero_location_margin = 7; 
  const int minimus_speed = 4; 

  int stickFBspeed;
  int stickFBspeed_sign;
  int stickLRspeed;
  
  stickFBspeed = motors_fb_pulse - zero_location;
  stickLRspeed = motors_lr_pulse - zero_location;
  
  float motorRspeed;
  float motorLspeed;
  
  if (stickFBspeed > 0)
  {
    stickFBspeed_sign = 1;
  }
  else if (stickFBspeed < 0)
  {
    stickFBspeed_sign = -1;
  }
  else
  {
    stickFBspeed_sign = 1;
  }
  motorRspeed = constrain(stickFBspeed-0.5*(stickLRspeed)*(stickFBspeed_sign), -500, 500);
  motorLspeed = constrain(stickFBspeed+0.5*(stickLRspeed)*(stickFBspeed_sign), -500, 500);

  int pwmA;
  int ain1;
  int ain2;
  String motorA_mode;
  int pwmB;
  int bin1;
  int bin2;
  String motorB_mode;

  //Motor A (right)
  if ( abs(motorRspeed) <  minimus_speed ) 
  {
    //stop - can change to brake if needed
    motorA_mode = "Stop";
    ain1 = LOW;
    ain2 = LOW; 
    pwmA = 0 ; 
  } 
  else if (motorRspeed > minimus_speed) 
  {
    //forward
    motorA_mode = "Forward";
    ain1 = HIGH;
    ain2 = LOW;      
    pwmA  = map(motorRspeed, 0, 500, 0, 255);
  }
  else 
  {
    //backward
    motorA_mode = "Backward";      
    ain1 = LOW;
    ain2 = HIGH;   
    pwmA  = map(motorRspeed, 0, -500, 0, 255);  
  }

  //Motor B (left)
  if ( abs(motorLspeed) <  minimus_speed ) 
  {
    //stop - can change to brake if needed
    motorB_mode = "Stop";
    bin1 = LOW;
    bin2 = LOW; 
    pwmB = 0 ; 
  } 
  else if (motorLspeed > minimus_speed) 
  {
    //forward
    motorB_mode = "Forward";
    bin1 = HIGH;
    bin2 = LOW;      
    pwmB  = map(motorLspeed, 0, 500, 0, 255);
  }
  else 
  {
    //backward
    motorB_mode = "Backward";      
    bin1 = LOW;
    bin2 = HIGH;   
    pwmB  = map(motorLspeed, 0, -500, 0, 255);  
  }
 
  digitalWrite(motor_ain1_out, ain1);
  digitalWrite(motor_ain2_out, ain2);
  analogWrite(motor_pwma_out, pwmA);
  digitalWrite(motor_bin1_out, bin1);
  digitalWrite(motor_bin2_out, bin2);
  analogWrite(motor_pwmb_out, pwmB);
  //digitalWrite(motors_stby_out, HIGH);
  
  int debug_motors ;
  debug_motors =1 ;
  if (debug_motors) {
    Serial.print("motors_fb_pulse : ");
    Serial.print(motors_fb_pulse);
    Serial.print(" | motors_lr_pulse :");
    Serial.print(motors_lr_pulse);
    Serial.println();
    Serial.print("stickFBspeed : ");
    Serial.print(stickFBspeed);
    Serial.print(" | stickLRspeed :");
    Serial.print(stickLRspeed);
    Serial.println();
    Serial.print("motorLspeed : ");
    Serial.print(motorLspeed);
    Serial.print(" | motorRspeed :");
    Serial.print(motorRspeed);
    Serial.println();
    Serial.print("Motor A (right) mode : ");
    Serial.print(motorA_mode);
    Serial.print(" | Motor A PWM:");
    Serial.println(pwmA);
    Serial.print("Motor B (left) mode : ");
    Serial.print(motorB_mode);
    Serial.print(" | Motor B PWM:");
    Serial.println(pwmB);
    delay(1000);
  }
}
