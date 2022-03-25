
/*******************************************************************************************************************
 * 
 * Provide a web based interface for Red/Green/Blue LED controller
 * AE December 2021   
 * 
 * Function:  To control red/green/blue LED via web interface using Wemos D1 mini microcontroller.  WiFi is set to operate  
 * in stand alone (Access Point) mode.  It does not connect to any existing WiFi network nor does it have access to 
 * the internet.  The WiFi connection only serves as a means of communicating between the RGB controller and an attached browser 
 * (e.g. phone, laptop)  
 * 
 * Inputs:  
 *    Three color values for PWM output ranging between 0 and 255
 *    Two servo motor inputs ranging in value between 0 and 180 (note SG90 servo motors traverse approx 90 degrees of angle)
 *    One on/off input to control laser pointer
 * 
 * Outputs:   
 *    PWM outputs for R/G/B LEDs 
 *    PWM outputs for servo motors
 *    digital output for laser pointer
 *    
 * Resources:     
 * Server concepts based on: https://raw.githubusercontent.com/techiesms/ESP8266-Series/master/WiFiAccessPoint/WiFiAccessPoint.ino
 * javascript adapted from example at https://www.iotstarters.com/wemos-d1-mini-web-server-based-servo-motor-control/
 * WiFi based on framework from: https://learn.sparkfun.com/tutorials/esp8266-thing-hookup-guide/example-sketch-ap-web-server
 * 
 * Key Changes
 * V4 - added laser
 * V7 - updated access point approach based on framework used at sparkfun site.  The previous approach was not accessible from some 
 *      IOS devices (security setting / HTML / header content?)  This seems to be resolved with V7.  Kept loop structure from V6 
 * V8 - cosmetic cleanup and documentation - researched use of EEPROM - not feasible with ESP8266
 * V9 - pickup current values of RBG, pan, tilt and laser so that page represents current state when invoked.  This involves updating the  
 *      widgets as well as the span (display) values
 * V9b - tested use of non-standard IP address
 * V9c - tested changing slider script from handling onchange to oninput events. [as delivered 1/25/22]
 * V9d - added encoding meta tag, this allows manual page refresh in IOS.   
 * 
 * TODO:  
 *  1. Set initial values on web page - completed with V9
 *  2. add domain name in place of dedicated IP address (using ESP8266mDNS.h)
 *  3. add secure login if used in future for critical applications
 *  4. add driver to allow array of LEDs to be used
 *  5. add captive portal for ease of use (if connected to AP automatically present page when browser is open)
 *  6. get current state of variables an set those vaues in HTML page before refresh - completed with V9
 *  7. save current state of variables in memory and restore those values on power up. also reflect those values in initial HTML page - V8 not feasible 
 *  8. add configuration options screen to provide for initial position and color values on power up and to switch between local and remote (DMX) control
 *  9. add bluetooth audio interface to drive colors (and position) based on audio input
 *  10. add DMX interface
 *  11. put variables into a struture to simplify storage
 *  12. Stylize sliders for better visibility / use
 *  13. Add infrared receiver so that values may be entered via TV remote
 *  14. Add favicon storing ico file in flash memory and using arduino IDE to upload file to flash
 *  
 *  Known Issues:
 *  1. slider values produce erroneous results when driven to extremeties.  Example - drive pan slider 
 *  from mid range to max right.  servo motor will achieve max right position then another event will be sent which moves the servo
 *  to a position short of max right.  This is an issue for both all sliders.  This behavior appears to be independent of the amount of 
 *  delay introduced following write to servo.  changing handler from onchange to oninput solves this problem but makes the UI 
 *  less responsive
 *  
 *******************************************************************************************************************/
  
#include <ESP8266WiFi.h>
#include <Servo.h>

// CONSTANTS
const int panServoPin     = D1;
const int analogOutBlue   = D2;
const int analogOutGreen  = D3;
const int analogOutRed    = D4;  
const int tiltServoPin    = D5;  
const int laserPin        = D6; 

const char *ssid = "RGB Demo";                  // name of wireless access point
const char *pw = "12345abcde";                  // password

// STRINGS
String lValueString   = String(0);
String pValueString   = String(0);
String tValueString   = String(0);
String redValueString = String(0);
String grnValueString = String(0);
String bluValueString = String(0);
String header;

// NUMERICS
int leftDelimiter         = 0;
int rightDelimiter        = 0;
long liLastPanServoTime   = 0;
long liLastTiltServoTime  = 0;
long liLastUpdateTime     = 0;

                                                  // once user input has been provided these values will always have the current
                                                  // user-provided values.  These values will be used to format the page for subsequent
                                                  // use
int iRedValue             = 10;                   // set initial value relatively dim.  Low values here represent low brightness 
int iGreenValue           = 10;                   // set initial value relatively dim.  Low values here represent low brightness  
int iBlueValue            = 10;                   // set initial value relatively dim.  Low values here represent low brightness  
int iPanValue             = 90;                   // set initial value to middle of range of motion
int iTiltValue            = 90;                   // set initial value to middle of range of motion
int iLaserValue           = 0;                    // set initial value to off

// OBJECTS
Servo panServo;
Servo tiltServo;
WiFiServer server(80);

void setup(){
  // *************************  initialize communication
  Serial.begin(115200);

  pinMode(analogOutRed,   OUTPUT);
  pinMode(analogOutGreen, OUTPUT);
  pinMode(analogOutBlue,  OUTPUT);
  pinMode(tiltServoPin,   OUTPUT);
  pinMode(panServoPin,    OUTPUT);
  pinMode(laserPin,       OUTPUT); 

  analogWriteRange(255);
  analogWriteFreq(2000);

  // turn off RGB LEDs
  analogWrite(analogOutRed,   255);
  analogWrite(analogOutGreen, 255);
  analogWrite(analogOutBlue,  255);

  // *** set pan and tilt values  Note SG90 servos have 90 degree range of motion
  // with input ranging from 0 to 180.  Servos will remain detached unless in use.
  panServo.attach(panServoPin);
  tiltServo.attach(tiltServoPin);
  panServo.write(iPanValue);
  tiltServo.write(iTiltValue); 
  delay(1500); 
  panServo.detach();
  tiltServo.detach();

  // *** set RGB Values (Note: using CA LEDs - high values = dim)  
  // display subset of range of colors
  #define STEP_SIZE 16
  for(int i = 255; i> 127; i-=STEP_SIZE){
    for(int j = 255; j> 127; j-=STEP_SIZE){
      for(int k = 255; k> 127; k-=STEP_SIZE){
        analogWrite(analogOutRed,   i);
        analogWrite(analogOutGreen, j);
        analogWrite(analogOutBlue,  k);
        delay(2);
      }
    }
  }

  // set initial RGB values to dim white 
  analogWrite(analogOutRed,   255 - iRedValue);
  analogWrite(analogOutGreen, 255 - iGreenValue);
  analogWrite(analogOutBlue,  255 - iBlueValue);
  
  // *** set laser to default value established above (off)
  digitalWrite(laserPin,iLaserValue); 

  // *************************  setup wifi
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, pw);

  // *************************  start server
  server.begin();
  Serial.println("Server started");
}

void loop(){
  WiFiClient client = server.available(); 
  if (client)
    { 
       /************************************************************************** 
       *  
       *  Three steps requred:
       *  1. setup and send page to client
       *  2. handle response from client
       *  3. once responses have been handled zero out header and 
       *     disconnect from server 
       *  
       ************************************************************************/

       // *******************************************  Step 1  - setup and send page to client
      String header = client.readStringUntil('\r');    
  
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/html");
      client.println("Connection: close");
      client.println();
      
      client.println("<!DOCTYPE html><html>");
      client.println("<meta charset='utf-8'>");                // V9d - added
      client.println("<head><meta name='viewport' content='width=device-width, initial-scale=1', user-scalable=no'>");  // v9d - lockout user scale on duobletap
      client.println("<title>RGB Blender</title>");

      // style      
      client.println("<style>body { text-align: center; font-family: 'Trebuchet MS', Arial; margin-left:auto; margin-right:auto; }");
      client.println(".headertext{ font-weight:bold; font-family:Arial; color: black; text-align: center; }");    
      client.println("</style>");
      
      client.println("</head><body><h2><u><div class = 'headertext'> RGB control</u></h2>");
      // pickup javascript functionality 
      client.println("<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js'></script>");

      // *************************************************** Red Slider
      // label and span to hold the selected value for display
      // pickup current value to load to span
      client.print("<h4>Red <span id='rSliderValue' style='color:red;'>");
      client.print(iRedValue);
      client.println("</span> </h4>");
      // slider
      // pickup current value so that page represents current state when refreshed 
      client.print("Off <input id='rSlider' type='range' min='0' max='255' step='1' value='");
      client.print(iRedValue);
      client.println("' style='width: 66%;' oninput='rSliderInput(this.value)'> Max");
      // slider functions
      client.println("<script> ");
      client.println("  function rSliderInput(val) {document.getElementById('rSliderValue').innerHTML = val; ");  // place value on client screen in span
      client.println("  $.get('/?rvalue=' + val + '&'); {Connection: close};}");                                  // place value in header to deliver to server
      client.println("</script>");  
      client.println("<br>");
      client.println("<br>");
  
      // *************************************************** Green Slider
      // label and span to hold the selected value for display
      // pickup current value to load to span
      client.print("<h4>Green <span id='gSliderValue' style='color:green;'>");
      client.print(iGreenValue);
      client.println("</span> </h4>");      
      // slider
      // pickup current value so that page represents current state when refreshed 
      client.print("Off <input id='gSlider' type='range' min='0' max='255' step='1' value='");
      client.print(iGreenValue);
      client.println("' style='width: 66%;' oninput='gSliderInput(this.value)'> Max");
      // slider functions
      client.println("<script> ");
      client.println("  function gSliderInput(val) {document.getElementById('gSliderValue').innerHTML = val; ");  // place value on client screen in span
      client.println("  $.get('/?gvalue=' + val + '&'); {Connection: close};}");                                  // place value in header to deliver to server
      client.println("</script>");   
      client.println("<br>");
      client.println("<br>");
        
      // *************************************************** Blue Slider
      // label and span to hold the selected value for display
      // pickup current value to load to span
      client.print("<h4>Blue <span id='bSliderValue' style='color:blue;'>");
      client.print(iBlueValue);
      client.println("</span> </h4>");      
      // slider
      // pickup current value so that page represents current state when refreshed 
      client.print("Off <input id='bSlider' type='range' min='0' max='255' step='1' value='");
      client.print(iBlueValue);
      client.println("' style='width: 66%;' oninput='bSliderInput(this.value)'> Max");
      // slider functions
      client.println("<script> ");
      client.println("  function bSliderInput(val) {document.getElementById('bSliderValue').innerHTML = val; ");  // place value on client screen in span
      client.println("  $.get('/?bvalue=' + val + '&'); {Connection: close};}");                                  // place value in header to deliver to server
      client.println("</script>"); 
      client.println("<br>");
      client.println("<br>");
      
      // *************************************************** pan servo motor slider
      // label and span to hold the selected value for display
      // pickup current value to load to span
      client.print("<h4>Pan position <span id='panSliderValue' style='color:black;'>");
      client.print(iPanValue);
      client.println("</span> </h4>");
      // slider
      // pickup current value so that page represents current state when refreshed       
      client.print("Left <input id='panSlider' type='range' min='0' max='180' step='1' value='");
      client.print(iPanValue);
      client.println("' style='width: 66%;' oninput='pSliderInput(this.value)'> Right");
      // slider functions
      client.println("<script> ");
      client.println("  function pSliderInput(val) {document.getElementById('panSliderValue').innerHTML = val; ");  // place value on client screen in span
      client.println("  $.get('/?pvalue=' + val + '&'); {Connection: close};}");                                    // place value in header to deliver to server
      client.println("</script>");   
      client.println("<br>");
      client.println("<br>");
      
      // *************************************************** tilt servo motor slider
      // label and span to hold the selected value for display
      client.print("<h4>Tilt position <span id='tiltSliderValue' style='color:black;'>");
      client.print(iTiltValue);
      client.println("</span> </h4>");
      // slider
      // pickup current value so that page represents current state when refreshed       
      client.print("Down <input id='tiltSlider' type='range' min='0' max='180' step='1' value='");
      client.print(iTiltValue);
      client.println("' style='width: 66%;' oninput='tSliderInput(this.value)'> Up");
      // slider functions
      client.println("<script> ");
      client.println("  function tSliderInput(val) {document.getElementById('tiltSliderValue').innerHTML = val; "); // place value on client screen in span
      client.println("  $.get('/?tvalue=' + val + '&'); {Connection: close};}");                                    // place value in header to deliver to server
      client.println("</script>");   
      client.println("<br>");
      client.println("<br>");

      // *************************************************** laser pointer control
      // label and span to hold the selected value for display
      //client.println("<h4>Laser pointer <span id='lSliderValue' style='color:black;'></span></h4>");
      // slider
      // pickup current value so that page represents current state when refreshed       
      client.print("Laser &nbsp &nbsp &nbsp Off <input id='lSlider' type='range' min='0' max='1' step='1' value='");
      client.print(iLaserValue);
      client.println("' style='width: 12%;' oninput='lSliderInput(this.value)'> On");
      // slider functions
      client.println("<script> ");
     // client.println("  function lSliderInput(val) {document.getElementById('lSliderValue').innerHTML = val; ");    // place value on client screen in span
      client.println("  function lSliderInput(val) { ");    // place value on client screen in span   
      client.println("  $.get('/?lvalue=' + val + '&'); {Connection: close};}");                                      // place value in header to deliver to server
      client.println("</script>"); 

      // close open elements from above
      client.println("</body></html>"); 
  
      // ******************************************* Step 2 -  Handle results - check for return of each possible element
      // display header contents for development / diagnosis
      Serial.println(header);     
      
      // 1/9/21 - accomodate common anode LED by inverting commanded output (slider value of 255 results in output value of zero)
      // red value returned?
      if(header.indexOf("rvalue=")>=0) 
        {
            leftDelimiter = header.indexOf('=');                                        // left boundary of return value
            rightDelimiter = header.indexOf('&');                                       // right boundary of return value
            redValueString = header.substring(leftDelimiter+1, rightDelimiter);
            iRedValue = redValueString.toInt();
            analogWrite(analogOutRed, 255 - iRedValue);                                 // invert output for common anode LED
            Serial.print("Red value: "); Serial.println(redValueString);                // display "normal" input vaule
        } 
  
      // green value returned?
      if(header.indexOf("gvalue=")>=0) 
        {
            leftDelimiter = header.indexOf('=');                                         // left boundary of return value
            rightDelimiter = header.indexOf('&');                                        // right boundary of return value
            grnValueString = header.substring(leftDelimiter+1, rightDelimiter);
            iGreenValue = grnValueString.toInt();
            analogWrite(analogOutGreen, 255 - iGreenValue);                              // invert output for common anode LED
            Serial.print("Green value: ");Serial.println(grnValueString);                // display "normal" input vaule 
        } 
  
      // blue value returned?
      if(header.indexOf("bvalue=")>=0) 
        {
            leftDelimiter = header.indexOf('=');                                         // left boundary of return value
            rightDelimiter = header.indexOf('&');                                        // right boundary of return value
            bluValueString = header.substring(leftDelimiter+1, rightDelimiter);
            iBlueValue = bluValueString.toInt();
            analogWrite(analogOutBlue, 255 - iBlueValue);                                // invert output for common anode LED
            Serial.print("Blue value: ");Serial.println(bluValueString);                 // display "normal" input vaule
        } 
  
      // pan position returned?
      if(header.indexOf("pvalue=")>=0) 
        {
            if(panServo.attached()== false) panServo.attach(panServoPin);
            leftDelimiter = header.indexOf('=');                                        // left boundary of return value
            rightDelimiter = header.indexOf('&');                                       // right boundary of return value
            pValueString = header.substring(leftDelimiter+1, rightDelimiter);
            iPanValue = pValueString.toInt();
            panServo.write(180 - iPanValue);
            delay(1);
            liLastPanServoTime = millis();                                              // get timestamp of most recent servo use, disconnect servo after x idle time
            Serial.print("pan value: ");Serial.println(pValueString.toInt());         
        }
         
      // tilt position returned?
      if(header.indexOf("tvalue=")>=0) 
        {
            if(tiltServo.attached()== false) tiltServo.attach(tiltServoPin);
            leftDelimiter = header.indexOf('=');                                        // left boundary of return value
            rightDelimiter = header.indexOf('&');                                       // right boundary of return value
            tValueString = header.substring(leftDelimiter+1, rightDelimiter);
            iTiltValue = tValueString.toInt();
            tiltServo.write(180 - iTiltValue);
            delay(1);
            liLastTiltServoTime = millis();                                             // get timestamp of most recent servo use, disconnect servo after x idle time
            Serial.print("tilt value: ");Serial.println(tValueString.toInt());      
        } 

      // laser pointer command returned?
      if(header.indexOf("lvalue=")>=0) 
        {
            leftDelimiter = header.indexOf('=');                                        // left boundary of return value
            rightDelimiter = header.indexOf('&');                                       // right boundary of return value
            lValueString = header.substring(leftDelimiter+1, rightDelimiter);
            iLaserValue = lValueString.toInt();
            digitalWrite(laserPin,iLaserValue); 
            Serial.print("laser value: ");Serial.println(lValueString);      
        } 
        
      // ******************************  Step 3  - zero out header and disconnect from server
      header = "";
      client.stop();
      Serial.println("");

    }

  // detach pan servo when not in use
  if((panServo.attached() and (millis() - liLastPanServoTime) > 1000)) {
    panServo.detach();
    Serial.println("disconnected idle pan servo");
  }

  // detach tilt servo when not in use
  if((tiltServo.attached() and (millis() - liLastTiltServoTime) > 1000)) {
    tiltServo.detach();
    Serial.println("disconnected idle tilt servo");
  }

}
