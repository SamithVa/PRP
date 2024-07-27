#include <math.h>

// Main Board

// PINS -----------------------------------------------------------------------------------

// RIGHT JOYSTICKS, TRACK SIGNAL: PWM
const byte right_x_pin = 12;
const byte right_y_pin = 11;

// LEFT JOYSTICKS, PROPELLER SIGNAL: PWM
const byte left_x_pin = 9;
const byte left_y_pin = 10;

// PUMP SIGNAL FROM RECEIVER : PWM
const byte pumpSignal = 6;

// TWO PUMPS WATER IN : DIGITAL
const byte pumpOut = 31;

// ONE PUMP WATER OUT : DIGITAL
const byte pumpIn = 33;

// Actuator 
const byte actuatorSignal = 5; // PWM

const byte actuatorA = 41; // DIGITAL
const byte actuatorB = 43; // DIGITAL

// END OF PINS -------------------------------------------------------------------------------

// CONSTANTS ---------------------------------------------------------------------------------------

const int trackMaxSpeed = 500;
const int trackMinSpeed = -500;

// Constants for RIGHT joystick range mapping
const int TrackMinRawValue = 700;
const int TrackMaxRawValue = 2300;
const int TrackDeadZoneThreshold = 50; // Threshold for joystick being "in the center"
const double RadToDeg = 180 / M_PI;    // Conversion factor from radians to degrees

// Constants for LEFT joystick range mapping
const int PropellerMinRawValue = 900;
const int PropellerMaxRawValue = 2100;
const int PropellerDeadZoneThreshold = 20; // Threshold for joystick being "in the center"
const int SmallMovementThreshold = 10;
const int NeutralPropellerSpeed = 90;

// Propeller PWM value range 0 to 80
const int propellerMaxSpeed = 150;
const int propellerMinSpeed = -150;

int right_raw_x = 0;
int right_raw_y = 0;

int left_raw_x = 0;
int left_raw_y = 0;

// END OF CONSTANTS ---------------------------------------------------------------------------------------

struct speed
{
    int Left = 0;
    int Right = 0;
};

speed calibrateTrackSignal(int raw_x, int raw_y);
speed calibratePropellerSignal(int raw_x, int raw_y);
speed trackReadCalibrate();
speed propellerReadCalibrate();
void sendSignals();
void pumps();
void actuator();

void setup()
{
    Serial.begin(9600);

    pinMode(left_x_pin, INPUT);
    pinMode(left_y_pin, INPUT);

    pinMode(right_x_pin, INPUT);
    pinMode(right_y_pin, INPUT);

    pinMode(pumpSignal, INPUT);
    pinMode(pumpIn, OUTPUT);
    pinMode(pumpOut, OUTPUT);

    pinMode(actuatorSignal, INPUT);
    pinMode(actuatorA, OUTPUT);
    pinMode(actuatorB, OUTPUT);
}

void loop()
{

    // Read joystick input : RIGHT JOYSTICK FOR TRAIL MOTOR
    speed track = trackReadCalibrate();

    // Read joystick input : LEFT JOYSTICK FOR PROPELLER
    speed propeller = propellerReadCalibrate();

    sendSignals(track, propeller);

    pumps();
    actuator();
    delay(20);
}

speed calibrateTrackSignal(int raw_x, int raw_y)
{
    // Serial.println(raw_x);
    // Serial.println(raw_y);

    int x = map(raw_x, TrackMinRawValue, TrackMaxRawValue, trackMinSpeed, trackMaxSpeed);
    int y = map(raw_y, TrackMinRawValue, TrackMaxRawValue, trackMinSpeed, trackMaxSpeed);

    speed ret;

    // Check if joystick is in the center
    if (abs(x) < TrackDeadZoneThreshold && abs(y) < TrackDeadZoneThreshold)
    {
        return ret; // Early return if in center, speeds remain zero
    }

    // Calculate turning coefficient
    if (x == 0)
        x = 1; // Prevent division by zero
    double degree = atan(static_cast<double>(abs(y)) / abs(x)) * RadToDeg;
    int diff = abs(x) - abs(y);
    int scaledSignal = round((-1 + (degree / 90.0) * 2) * abs(diff));
    int unscaledSignal = max(abs(x), abs(y));

    // Set speeds based on the quadrant of the joystick
    if ((x >= 0 && y >= 0) || (x < 0 && y < 0))
    {
        ret.Right = scaledSignal;
        ret.Left = unscaledSignal;
    }
    else
    {
        ret.Right = unscaledSignal;
        ret.Left = scaledSignal;
    }

    // Reverse speeds if y is negative
    if (y < 0)
    {
        ret.Left = -ret.Left;
        ret.Right = -ret.Right;
    }

    // Constrain outputs to maximum and minimum speeds
    ret.Left = constrain(ret.Left, trackMinSpeed, trackMaxSpeed);
    ret.Right = constrain(ret.Right, trackMinSpeed, trackMaxSpeed);

    return ret;
}

speed calibratePropellerSignal(int raw_x, int raw_y)
{
    // Mapping raw input to speed range
    int x = map(raw_x, PropellerMinRawValue, PropellerMaxRawValue, 0, 180);
    int y = map(raw_y, PropellerMinRawValue, PropellerMaxRawValue, 0, 180);

    speed propeller;

    // Pause or neutral
    if (abs(x - 90) < SmallMovementThreshold && abs(y - 90) < SmallMovementThreshold) {
        propeller.Left = 90;
        propeller.Right = 90;
    }
    // Forward
    else if (y > 90 + SmallMovementThreshold) {
        propeller.Left = 150;
        propeller.Right = 150;
    }
    // Backward
    else if (y < 90 - SmallMovementThreshold) {
        propeller.Left = 30;
        propeller.Right = 30;
    }
    // Forward-left
    else if (x < 90 - SmallMovementThreshold && y > 90 + SmallMovementThreshold) {
        propeller.Left = 120;
        propeller.Right = 150;
    }
    // Forward-right
    else if (x > 90 + SmallMovementThreshold && y > 90 + SmallMovementThreshold) {
        propeller.Left = 150;
        propeller.Right = 120;
    }
    // Backward-left
    else if (x < 90 - SmallMovementThreshold && y < 90 - SmallMovementThreshold) {
        propeller.Left = 30;
        propeller.Right = 60;
    }
    // Backward-right
    else if (x > 90 + SmallMovementThreshold && y < 90 - SmallMovementThreshold) {
        propeller.Left = 60;
        propeller.Right = 30;
    }
    // Left only
    else if (x < 90 - SmallMovementThreshold) {
        propeller.Left = 60;
        propeller.Right = 120;
    }
    // Right only
    else if (x > 90 + SmallMovementThreshold) {
        propeller.Left = 120;
        propeller.Right = 60;
    }

    // Constrain outputs to valid speed range
    propeller.Left = constrain(propeller.Left, 0, 180);
    propeller.Right = constrain(propeller.Right, 0, 180);

    return propeller;
}

speed trackReadCalibrate()
{
    right_raw_x = pulseIn(right_x_pin, HIGH);
    right_raw_y = pulseIn(right_y_pin, HIGH);
    speed track = calibrateTrackSignal(right_raw_x, right_raw_y);
    return track;
}

speed propellerReadCalibrate()
{
    left_raw_x = pulseIn(left_x_pin, HIGH);
    left_raw_y = pulseIn(left_y_pin, HIGH);
    // Serial.println(left_raw_x);
    // Serial.println(left_raw_y);

    // Calculate track speeds
    // speed propeller;
    // Calculate properller speeds
    speed propeller = calibratePropellerSignal(left_raw_x, left_raw_y);
    return propeller;
}

void sendSignals(speed track, speed propeller)
{
    // Send the signal to Arduino UNO
    Serial.print(track.Left);
    Serial.print(",");
    Serial.print(track.Right);
    Serial.print(",");
    Serial.print(propeller.Left);
    Serial.print(",");
    Serial.print(propeller.Right);
    Serial.println();
}

// PUMPS --------------------------------------------
void pumps()
{
    // Read the pump button signal
    int pumpS = pulseIn(pumpSignal, HIGH);

    // Serial.println(pumpS);
    if (pumpS < 900)
    {
        // Pump water into the tank
        digitalWrite(pumpOut, LOW);
        digitalWrite(pumpIn, HIGH);
    }
    else if (pumpS < 1500)
    {
        digitalWrite(pumpOut, LOW);
        digitalWrite(pumpIn, LOW);
    }
    else
    {   // Pump water outside the tank
        digitalWrite(pumpOut, HIGH);
        digitalWrite(pumpIn, LOW);
    }
}
// END OF PUMPS ------------------------------------

// ACTUATOR CONTROL ----------------------------------
void actuator(){
  int value = pulseIn(actuatorSignal, HIGH); // Range : 800 - 2200
  // Serial.println(value);
  if(value > 800 && value < 1000){ // COMPRESS
    digitalWrite(actuatorA, HIGH);
    digitalWrite(actuatorB, LOW);
  }
  else if(value > 1700){ // EXTEND
    digitalWrite(actuatorA, LOW);
    digitalWrite(actuatorB, HIGH);
  }
  else{ 
    digitalWrite(actuatorA, LOW);
    digitalWrite(actuatorB, LOW);
  }
}

// END OF ACTUATOR CONTROL

