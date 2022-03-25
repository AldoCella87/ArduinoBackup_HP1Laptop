//Activity 7: HelloWorld4
// Hello World4
// This program flashes an LED for a duration specified via analog input
// connect pin 9 of microcontroller to anode of LED
// connect cathode of LED to a 1000 ohm resistor
// connect the other end of the 1000 ohm resistor to ground
// connect one of the outside legs of the potentiometer to +5V
// connect the other outside leg of the potentiometer to ground
// connect the center leg of the potentiometer to the Analog input 0
// display the analog value on the serial monitor
// remove display of on/off state to serial monitor in previous example

int iAnalogValue = 0;   // declare an integer variable to hold the analog value 

void setup() {
  // put your setup code here, to run once:
  pinMode(9,OUTPUT);      // assign pin 9 as an output
  Serial.begin(9600);     // setup the serial port for communication to the monitor
  Serial.println("Starting HelloWorld4");
}

void loop() {
  // put your main code here, to run repeatedly:

  iAnalogValue = analogRead(0);   // read analog value from analog input zero
                                  // and store in the variable iAnalogValue
                                  // analog values are between 0 and 1023
                                  
  Serial.println(iAnalogValue);   // display analog value to serial monitor

  // turn LED on 
  digitalWrite(9, HIGH);          // set output high. This will turn LED on
  delay(iAnalogValue);            // wait for time specified by analog input

  // turn LED off
  digitalWrite(9, LOW);           // set output low. This will turn LED off
  delay(iAnalogValue);            // wait for time specified by analog input
  
}
