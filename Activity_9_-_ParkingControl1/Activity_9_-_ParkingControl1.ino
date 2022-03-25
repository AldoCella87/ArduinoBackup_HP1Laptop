// Activity 9: ParkingControl1
// ParkingControl1
/* Ping))) Sensor

   This sketch reads a PING))) ultrasonic rangefinder and returns the
   distance to the closest object in range. To do this, it sends a pulse
   to the sensor to initiate a reading, then listens for a pulse
   to return.  The length of the returning pulse is proportional to
   the distance of the object from the sensor.

   The circuit:
  * +V connection of the PING))) attached to +5V
  * GND connection of the PING))) attached to ground
  * SIG connection of the PING))) attached to digital pin 7

   http://www.arduino.cc/en/Tutorial/Ping

   created 3 Nov 2008
   by David A. Mellis
   modified 30 Aug 2011
   by Tom Igoe
   Modified 01 DEC 2017 AE - added RYG Indicators, saved as ParkingControl, 
   This example code is in the public domain.
 */

// this constant won't change.  It's the pin number
// of the sensor's output:
const int pingPin = 7;

void setup() {
  // initialize serial communication:
  Serial.begin(9600);

  // added for parking indicator
  pinMode(8,OUTPUT);              // set pin 8 to output mode for red LED
  pinMode(9,OUTPUT);              // set pin 9 to output mode for yellow LED
  pinMode(10,OUTPUT);             // set pin 10 to output mode for green LED
  
}
void loop() {
  // establish variables for duration of the ping,
  // and the distance result in inches and centimeters:
  long duration, inches, cm;

  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);

  // The same pin is used to read the signal from the PING))): a HIGH
  // pulse whose duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(pingPin, INPUT);
  duration = pulseIn(pingPin, HIGH);

  // convert the time into a distance
  inches = microsecondsToInches(duration);
  cm = microsecondsToCentimeters(duration);

  Serial.print(inches);
  Serial.print("in, ");
  Serial.print(cm);
  Serial.print("cm");

  if (inches > 16)
  // distance greater than 16 - signal yellow = proceed with caution
  {
      // display yellow
      digitalWrite(8, LOW);           // turn red LED off   
      digitalWrite(9, HIGH);          // turn yellow LED on   
      digitalWrite(10, LOW);          // turn green LED off  
      Serial.print(" *** YELLOW");    // display message to serial monitor
  }
  else if(inches < 10)
  // distance less than 10 - back up before you hit something!
  {
      // display red
      digitalWrite(8, HIGH);           // turn red LED on   
      digitalWrite(9, LOW);            // turn yellow LED off  
      digitalWrite(10, LOW);           // turn green LED off
      Serial.print(" *** RED");        // display message to serial monitor   
  }
  else 
  // Juuuust right - stop anywhere within this zone
  {
      // display green
      digitalWrite(8, LOW);            // turn red LED off   
      digitalWrite(9, LOW);            // turn yellow LED off  
      digitalWrite(10, HIGH);          // turn green LED on
      Serial.print(" *** GREEN");      // display message to serial monitor   
  }

  //the two lines below are part of the original ping program
  Serial.println();                    // move cursor to next line
  delay(100);
}

long microsecondsToInches(long microseconds) {
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  // The speed of sound is 1130 ft/sec or 74 microseconds per inch.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds) {
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}
