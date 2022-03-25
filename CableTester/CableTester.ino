/*
  Button

  Turns on and off a light emitting diode(LED) connected to digital pin 13,
  when pressing a cable under test attached to pin 2.

  The circuit:
  - LED attached from pin 13 to ground
  - cable under test attached to pin 2 from +5V
  - 10K resistor attached to pin 2 from ground

  - Note: on most Arduinos there is already an LED on the board
    attached to pin 13.

  created 2005
  by DojoDave <http://www.0j0.org>
  modified 30 Aug 2011
  by Tom Igoe

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Button

  1/2020 - modified as a simple cable tester - AE
  
*/

// constants won't change. They're used here to set pin numbers:
const int TestInputPin = 8;     // the number of the cable under test pin
const int SonalertPin =  3;      // the number of the Sonalert pin

// variables will change:
int TestInputState = 0;         // variable for reading the cable under test status

void setup() {
  // initialize the LED pin as an output:
  pinMode(SonalertPin, OUTPUT);
  // initialize the cable under test pin as an input:
  pinMode(TestInputPin, INPUT_PULLUP);
}

void loop() {
  // read the state of the cable under test value:
  TestInputState = digitalRead(TestInputPin);

  // check if the cable under test is pressed. If it is, the TestInputState is HIGH:
  if (TestInputState == HIGH) {
    // turn LED on:
    digitalWrite(SonalertPin, HIGH);
    delay(50);
  } else {
    // turn LED off:
    digitalWrite(SonalertPin, LOW);
  }
}
