/* simulate firelight 
 * 12/19/19 AE
 */

// INCLUDES
                                         
// CONSTANTS

  
const int analogOutRed = 3;  
const int analogOutGreen = 5;  
const int analogOutBlue = 6;  

// VARIABLES


// variables to hold output values.  Range is 0 to 255
int i_RedValue = 0;
int i_GreenValue = 0;
int i_BlueValue = 0;
 

// SETUP - runs one time
void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);

  // set output pin mode
  pinMode(analogOutRed, OUTPUT);
  pinMode(analogOutGreen, OUTPUT);
  pinMode(analogOutBlue, OUTPUT);

}


// Loop - runs continuously
void loop() {

  //  using common anode RGB LED so low PWM gives maximum brightness
  i_RedValue = random(247, 251);
  i_GreenValue = random(253, 254);
  i_BlueValue = 255;
  
  // set the analog out value:
  analogWrite(analogOutRed, i_RedValue);
  analogWrite(analogOutGreen, i_GreenValue);
  analogWrite(analogOutBlue, i_BlueValue);

  // display results on serial monitor
  Serial.print("Red Value: ");
  Serial.print(i_RedValue);
  Serial.print("     Green Value: ");
  Serial.print(i_GreenValue);
  Serial.print("     Blue Value: ");
  Serial.print(i_BlueValue);
  Serial.println();

  
  // wait 2 milliseconds before the next loop for the analog-to-digital
  // converter to settle after the last reading:
  delay(random(2,900));
}
