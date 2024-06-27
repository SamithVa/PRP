#include <math.h>

// Main Board

// PINS -----------------------------------------------------------------------------------

// LEFT AND RIGHT JOYSTICKS : PWM
const byte right_x_pin = 11;
const byte right_y_pin = 10;
const byte left_y_pin = 9;
const byte left_x_pin = 8;

// PUMP SIGNAL FROM RECEIVER : PWM
const byte pumpSignal = 6;

// TWO PUMPS WATER IN : DIGITAL
const byte pumpOut = 28;

// ONE PUMP WATER OUT : DIGITAL
const byte pumpIn = 26;

// END OF PINS -------------------------------------------------------------------------------

// CONSTANTS ---------------------------------------------------------------------------------------

const int trackMaxSpeed = 500;
const int trackMinSpeed = -500;

// Propeller PWM value range 0 to 80
const int propellerMaxSpeed = 120;
const int propellerMinSpeed = -120;

// END OF CONSTANTS ---------------------------------------------------------------------------------------

struct speed{
  int Left = 0;
  int Right = 0;
};

speed calibrateTrackSignal(int raw_x, int raw_y){
  int x = map(raw_x, 1140, 1850, trackMinSpeed, trackMaxSpeed);
  int y = map(raw_y, 1140, 1850, trackMinSpeed, trackMaxSpeed);

  speed ret;

  // IF JOYSTICK IN THE CENTER
  if(x < 50 && x > -50 && y < 50 && y > -50){
    return ret;
  }

  // TURNING COEFFICIENT CALCULATION
  double degree = atan(fabs(y / x)) * 180 / M_PI;
  int scaledSignal = round((-1 + (degree / 90) * 2) * fabs((fabs(x) - fabs(y))));
  int unscaledSignal = max(fabs(x), fabs(y));

  // SET BASED ON CORRESPONDING QUADRANT
  if((x >= 10 && y >= 10) || (x <= -10 && y <= -10)){
    ret.Left = scaledSignal; 
    ret.Right = unscaledSignal;  
  }
  else {
    ret.Left = unscaledSignal;  
    ret.Right = scaledSignal;  
  }

  if(y < 0){
    ret.Left = -ret.Left;  // Reverse for negative y-axis
    ret.Right = -ret.Right;  // Reverse for negative y-axis
  }
  return ret;
}


speed calibratePropellerSignal(int raw_x, int raw_y) {
  int x = map(raw_x, 1220, 1800, propellerMinSpeed, propellerMaxSpeed);
  int y = map(raw_y, 1050, 1850, propellerMinSpeed, propellerMaxSpeed);

  speed ret;

  // Checking if the joystick is in the center area
  if (abs(x) < 20 && abs(y) < 20) {
    ret.Left = 90;  // Middle value for stop or neutral position
    ret.Right = 90;
    return ret;
  }
  // TURNING COEFFICIENT CALCULATION
  double degree = atan(fabs(y / x)) * 180 / M_PI;
  int scaledSignal = round((-1 + (degree / 90) * 2) * fabs((fabs(x) - fabs(y))));
  int unscaledSignal = max(fabs(x), fabs(y));

  // SET BASED ON CORRESPONDING QUADRANT
  if((x >= 10 && y >= 10) || (x <= -10 && y <= -10)){
    ret.Left = scaledSignal; 
    ret.Right = unscaledSignal;  
  }
  else {
    ret.Left = unscaledSignal;  
    ret.Right = scaledSignal;  
  }

  if(y < 0){
    ret.Left = 90-ret.Left;  // Reverse for negative y-axis
    ret.Right = 90-ret.Right;  // Reverse for negative y-axis
  }

  ret.Left = constrain(ret.Left, 0, propellerMaxSpeed);
  ret.Right = constrain(ret.Right, 0, propellerMaxSpeed);

  return ret;
  
}


void setup() {
  Serial.begin(9600);

  pinMode(left_x_pin,  INPUT);
  pinMode(left_y_pin,  INPUT);

  pinMode(right_x_pin,  INPUT);
  pinMode(right_y_pin,  INPUT);

  pinMode(pumpSignal, INPUT);
  pinMode(pumpIn, OUTPUT);
  pinMode(pumpOut, OUTPUT);
}
void loop() {

  // Read joystick input : RIGHT JOYSTICK FOR TRAIL MOTOR
  int right_raw_x = pulseIn(right_x_pin, HIGH);
  int right_raw_y = pulseIn(right_y_pin, HIGH);


  // Read joystick input : LEFT JOYSTICK FOR PROPELLER
  int left_raw_x = pulseIn(left_x_pin, HIGH);
  int left_raw_y = pulseIn(left_y_pin, HIGH);
  
  // Calculate track speeds
  speed trail = calibrateTrackSignal(right_raw_x, right_raw_y);

  // Calculate properller speeds
  speed propeller = calibratePropellerSignal(left_raw_x, left_raw_y);

  // Send the signal to Arduino UNO
  Serial.print(trail.Left);
  Serial.print(",");
  Serial.print(trail.Right);
  Serial.print(",");
  Serial.print(propeller.Left);
  Serial.print(",");
  Serial.print(propeller.Right);
  Serial.println();


// PUMPS --------------------------------------------
  int pumpS = pulseIn(pumpSignal, HIGH);
  // Serial.println(value);
  if(pumpS < 900){
    digitalWrite(pumpOut, LOW);
    digitalWrite(pumpIn, HIGH);
  }
  else if(pumpS < 1500){
    digitalWrite(pumpOut, LOW);
    digitalWrite(pumpIn, LOW);
  }
  else{
    digitalWrite(pumpOut, HIGH);
    digitalWrite(pumpIn, LOW);
  }
 
// END OF PUMPS ------------------------------------

}


 



