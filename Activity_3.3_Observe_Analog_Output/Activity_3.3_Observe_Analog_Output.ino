// Activity 3.3 Observe Analog Output
// CreateAnalogSignal
// This program generates a range of analog signals so that the output may be 
// observed by participants
// This example is intended to be as simple as possible in order to illustrate the fundamentals
// it may be refined as desired. 

void setup() {
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
  // Valid analog values range from 0 to 255.  A value of zero produces a digital signal which is in the off 
  // state 100% of the time.  A value of 255 produces a digital signal which is in the on state 100% of the time
  // values in between these two extremes produce a pulse with a pulse width proportionately between the extremes

  analogWrite(9, 0);            // write a value of zero to pin 9
  delay(4000);                  // give the user time to view the output

  analogWrite(9, 32);           // write a value of 32 to pin 9
  delay(4000);                  // give the user time to view the output

  analogWrite(9, 64);           // write a value of 64 to pin 9
  delay(4000);                  // give the user time to view the output

  analogWrite(9, 128);          // write a value of 128 to pin 9
  delay(4000);                  // give the user time to view the output

  analogWrite(9, 255);          // write a value of 255 to pin 9
  delay(4000);                  // give the user time to view the output 
}
