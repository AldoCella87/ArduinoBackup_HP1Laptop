
// pins for RGB LED
int RedLED = 3;                   // the PWM pin the red LED is attached to
int GreenLED = 5;                 // the PWM pin the green LED is attached to
int BlueLED = 6;                  // the PWM pin the blue LED is attached to



  int iRedSensorValue = 0;         // holds value returned by sensor 1
  int iGreenSensorValue = 0;         // holds value returned by sensor 2
  int iBlueSensorValue = 0;         // holds value returned by sensor 3


void setup() {

  Serial.begin(9600);             // initialize serial communication
  // pins for RGB LED
  pinMode(RedLED, OUTPUT);  
  pinMode(GreenLED, OUTPUT);  
  pinMode(BlueLED, OUTPUT);
}

void loop()
{

  // read three analog values from analog inputs
  iRedSensorValue = analogRead(A0) / 4; // get sensor 1 value
  iGreenSensorValue = analogRead(A1) / 4; // get sensor 2 value
  iBlueSensorValue = analogRead(A2) / 4; // get sensor 3 value

  // display the analog values on the serial monitor
  Serial.print("Red Sensor Value: ");
  Serial.println(iRedSensorValue);
  
  Serial.print("Green Sensor Value: ");
  Serial.println(iGreenSensorValue);
  
  Serial.print("Blue Sensor Value: ");
  Serial.println(iBlueSensorValue);

  Serial.println();
  /*****************
   // display 0000red
  analogWrite(RedLED, 255);
  analogWrite(GreenLED, 0);
  analogWrite(BlueLED, 0);
  delay(2000);
  // display green
  analogWrite(RedLED, 0);
  analogWrite(GreenLED, 255);
  analogWrite(BlueLED, 0);
  delay(2000);
  // display blue
  analogWrite(RedLED, 0);
  analogWrite(GreenLED, 0);
  analogWrite(BlueLED, 255);
  delay(2000);
  // display my custom color
  analogWrite(RedLED, 200);
  analogWrite(GreenLED, 60);
  analogWrite(BlueLED, 40);
  delay(2000);
  // turn off all LEDs
  analogWrite(RedLED, 0);
  analogWrite(GreenLED, 0);
  analogWrite(BlueLED, 0);
  delay(1000);
  *********************/
  // display color based on analog input values
  analogWrite(RedLED, iRedSensorValue);
  analogWrite(GreenLED, iGreenSensorValue);
  analogWrite(BlueLED, iBlueSensorValue);
  delay(30);

}
