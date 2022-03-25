// Activity 4:  HelloWorld1 – Basic Hello World
// Hello World1
// This program flashes an LED
// connect pin 9 of microcontroller to anode of LED
// connect cathode of LED to a 1000 ohm resistor
// connect the other end of the 1000 ohm resistor to ground

void setup() {
  // put your setup code here, to run once:
  pinMode(9,OUTPUT);              // assign pin 9 as an output

}

void loop() {
  // put your main code here, to run repeatedly:
  
  // turn LED on for one second
  digitalWrite(9, HIGH);          // set output high. This will turn LED on
  delay(1000);                    // wait one second

  // turn LED off for one second
  digitalWrite(9, LOW);           // set output low. This will turn LED off
  delay(1000);                    // wait one second
  
}
