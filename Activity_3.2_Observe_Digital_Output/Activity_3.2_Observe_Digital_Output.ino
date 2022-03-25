// Activity 3.2 Observe Digital Output
// CreatDigitalSignal
// This program generates a digital signal so that the output may be 
// observed by participants
// This example is intended to be as simple as possible in order to illustrate the fundamentals
// it may be refined as desired. 

void setup() {
  // put your setup code here, to run once:
  pinMode(9, OUTPUT);             // establish pin 9 as an output

}

void loop() {
  // put your main code here, to run repeatedly:

  digitalWrite(9, LOW);           // set pin 9 to low logic state (near zero volts)
  delay(2000);                    // wait 2 seconds 

  digitalWrite(9, HIGH);          // set pin 9 to high logic state (near five volts)
  delay(1000);                    // wait 1 second  
}
