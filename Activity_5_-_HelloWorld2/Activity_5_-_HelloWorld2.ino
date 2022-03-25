// Activity 5:  HelloWorld2
// Hello World2
// This program flashes an LED and displays the LED state in the serial monitor
// connect pin 9 of microcontroller to anode of LED
// connect cathode of LED to a 1000 ohm resistor
// connect the other end of the 1000 ohm resistor to ground

void setup() {
  // put your setup code here, to run once:
  pinMode(9,OUTPUT);              // assign pin 9 as an output
  Serial.begin(9600);             // setup the serial port for communication to the monitor
  Serial.println("Starting HelloWorld2");
}

void loop() {
  // put your main code here, to run repeatedly:

  // turn LED on for one second
  digitalWrite(9, HIGH);          // set output high. This will turn LED on
  Serial.println("LED ON");       // display status to serial monitor
  delay(1000);                    // wait one second

  // turn LED off for one second
  digitalWrite(9, LOW);           // set output low. This will turn LED off
  Serial.println("LED OFF");      // display status to serial monitor
  delay(1000);                    // wait one second
  
}
