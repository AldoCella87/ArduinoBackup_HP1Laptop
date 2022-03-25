// Activity 3.1 Observe PWM Output
// CreatePWMSignal
// This program generates a PWM signal of varying duty cycles
// so that the output may be observed by participants
// This example is intended to be as simple as possible in order to illustrate the fundamentals
// it may be refined as desired. 

#include <Servo.h>            // incorporate servo support into the program

Servo MyServo;                // create a servo object for use below


void setup() {
  // put your setup code here, to run once:
  MyServo.attach(9);          // Identify which pin the servo signal should be sent to
                              // connect the oscilloscope probe to this pin
}

void loop() {

  MyServo.write(0);           // Write PWM signal corresponding to zero degrees
  delay(4000);                // allow time for user to view signal

  MyServo.write(90);          // Write PWM signal corresponding to 90 degrees
  delay(4000);                // allow time for user to view signal

  MyServo.write(180);         // Write PWM signal corresponding to 180 degrees
  delay(4000);                // allow time for user to view signal

}
