#include <Servo.h> 

Servo ESC;

 
void setup() { // 
  ESC.attach(2,1000,2000); // (pin, min pulse width, max pulse width in microseconds) 
} 
void loop() { 
  int speed = 20;
  // potValue = map(potValue, 0, 1023, 0, 180); 
  ESC.write(speed); // Send the signal to the ESC 
}