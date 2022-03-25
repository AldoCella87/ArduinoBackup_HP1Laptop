// Activity 12: BTServo
// BTServo
// Control servo motor via Bluetooth connection
// This example is intended to be as simple as possible in order 
// to illustrate the fundamentals.  Essentially this sketch receives
// one of two values over a Bluetooth connection and sets a PWM 
// output accordingly.

char data = 0;                //Variable for storing received data

#include <Servo.h>
Servo MyServo;                // create servo object to control a servo
int iPos = 10;                // starting position for the servo in degrees
void setup() 
{
  Serial.begin(9600);         // set speed of serial port
  pinMode(13, OUTPUT);        // set digital pin 13 as output
  MyServo.attach(9);          // associate pin 9 with servo object
  MyServo.write(iPos);        // set servo to the default position (10 degrees)    
}
void loop()
{
  if(Serial.available() > 0) // Check to see if serial data has been received
  {
    data = Serial.read();      // store incoming data
    Serial.println(data);      // display incoming data in serial monitor
    if(data == '1')            // check incoming data - move servo if it is a 1
    {
        iPos = 160;            // set desired servo location to 160 degrees
        digitalWrite(13, HIGH);// set internal LED on
    }
    else if(data == '0')        // Value was not 1, check for value of zero.
    {
        iPos = 10;              // set desired servo location to 10 degrees 
        digitalWrite(13, LOW);  // turn internal LED off
    }
    
    MyServo.write(iPos); // command the servo to the position established above
  } // end if(Serial.available()...                         
 }
