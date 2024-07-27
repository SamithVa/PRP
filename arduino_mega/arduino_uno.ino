#include <AccelStepper.h>
#include <Servo.h>

// TRAIL MOTORS PINS
const byte dirL = 2;  
const byte stepL = 3; 

const byte dirR = 7; 
const byte stepR = 6;

// PROPELLERS PINS : PWM
const byte prop_L = 10;
const byte prop_R = 11;

// AccelStepper setup
AccelStepper stepper1(1, stepL, dirL);
AccelStepper stepper2(1, stepR, dirR);

// Servo setup
Servo propeller_L;     
Servo propeller_R;

// Speed variables
int speed_TL = 0;
int speed_TR = 0;
int speed_PL = 90;
int speed_PR = 90;

void setup() {  
  Serial.begin(9600);  
  stepper1.setMaxSpeed(500); 
  stepper2.setMaxSpeed(500); 

  propeller_L.attach(prop_L, 1000, 2000); // 50 Hz
  propeller_R.attach(prop_R, 1000, 2000); 
}
 
void loop() {
  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');  // Read data until newline
    int commaIndex1 = data.indexOf(',');
    int commaIndex2 = data.indexOf(',', commaIndex1 + 1);
    int commaIndex3 = data.indexOf(',', commaIndex2 + 1);

    if (commaIndex1 != -1 && commaIndex2 != -1 && commaIndex3 != -1) {
      // Parse each part of the data string
      String raw_tl_str = data.substring(0, commaIndex1);
      String raw_tr_str = data.substring(commaIndex1 + 1, commaIndex2);
      String raw_pl_str = data.substring(commaIndex2 + 1, commaIndex3);
      String raw_pr_str = data.substring(commaIndex3 + 1);

      // Convert strings to integers
      speed_TL = raw_tl_str.toInt();
      speed_TR = raw_tr_str.toInt();
      speed_PL = raw_pl_str.toInt();
      speed_PR = raw_pr_str.toInt();
    }
  }

  // Control propellers using PWM signals
  propeller_L.write(speed_PL);
  propeller_R.write(speed_PR);

  // Control trail motors using stepper speeds
  stepper1.setSpeed(speed_TL);
  stepper1.runSpeed();
  stepper2.setSpeed(speed_TR);
  stepper2.runSpeed();
}
