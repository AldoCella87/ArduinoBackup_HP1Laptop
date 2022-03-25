
/*******************************************************************************************************************
 * 
 * Provide a web based interface for Red/Green/Blue LED controller
 * AE January 2022   
 * 
 * Function:  To control red/green/blue neopixel RGB LED array via web interface using Wemos D1 mini microcontroller.  WiFi is set to operate  
 * in stand alone (Access Point) mode.  It does not connect to any existing WiFi network nor does it have access to 
 * the internet.  The WiFi connection only serves as a means of communicating between the RGB controller and an attached browser 
 * (e.g. phone, laptop)  
 * 
 * Inputs:  
 *    Three color values for output ranging between 0 and 255
 *  
 * Outputs:   
 *    single data line to neopixel
 *    
 * Resources:     
 * Server concepts based on: https://raw.githubusercontent.com/techiesms/ESP8266-Series/master/WiFiAccessPoint/WiFiAccessPoint.ino
 * javascript adapted from example at https://www.iotstarters.com/wemos-d1-mini-web-server-based-servo-motor-control/
 * WiFi based on framework from: https://learn.sparkfun.com/tutorials/esp8266-thing-hookup-guide/example-sketch-ap-web-server
 * 
 * Key Changes
 * 
 * TODO:  
 *   add domain name in place of dedicated IP address (using ESP8266mDNS.h)
 *   add captive portal for ease of use (if connected to AP automatically present page when browser is open)
 *   get current state of variables an set those vaues in HTML page before refresh - completed with V9
 *   save current state of variables in memory and restore those values on power up. also reflect those values in initial HTML page - V8 not feasible 
 *   add configuration options screen to provide for initial position and color values on power up and to switch between local and remote (DMX) control
 *   add bluetooth audio interface to drive colors (and position) based on audio input
 *   put variables into a struture to simplify storage
 *   Stylize sliders for better visibility / use
 *   Add infrared receiver so that values may be entered via TV remote
 *   Add favicon storing ico file in flash memory and using arduino IDE to upload file to flash
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
 

// CONSTANTS

const char *ssid = "Petrolux";                  // name of wireless access point
const char *pw = "12345abcde";                  // password

// STRINGS   TODO - replace with a single work field
String sValueString   = String(0);
String header;

// NUMERICS
int leftDelimiter         = 0;
int rightDelimiter        = 0;
                                                  // once user input has been provided these values will always have the current
                                                  // user-provided values.  These values will be used to format the page for subsequent
                                                  // use
int iRedValue             = 127;                  
int iGreenValue           = 127;                  
int iBlueValue            = 127;                  
int iIntensityValue       = 10;                   // set low default brightness for neopixel
int iSW1Value             = 0;  
int iSW2Value             = 0;
int iSW3Value             = 0;

// OBJECTS
WiFiServer server(80);

// neopixel
#include <Adafruit_NeoPixel.h>
#define DATAPIN      D2
#define NUMPIXELS     7
Adafruit_NeoPixel pixels(NUMPIXELS, DATAPIN, NEO_GRB + NEO_KHZ800);

 

void setup(){
  // *************************  initialize communication
  Serial.begin(115200);


  // set initial intensity for neopixel
  iIntensityValue = 10;

  // *************************  setup wifi
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, pw);

  // *************************  start server
  server.begin();
  Serial.println("Server started");

  pixels.begin();

  // illustrate neopixel colors
  write_fixed_colors();
  delay(2000);
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
      client.println("<title>Petrolux</title>");

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
        
      // *************************************************** Intensity Slider
      // label and span to hold the selected value for display
      // pickup current value to load to span
      client.print("<h4>Intensity <span id='iSliderValue' style='color:black;'>");
      client.print(iIntensityValue);
      client.println("</span> </h4>");      
      // slider
      // pickup current value so that page represents current state when refreshed 
      client.print("Off <input id='iSlider' type='range' min='0' max='255' step='1' value='");
      client.print(iIntensityValue);
      client.println("' style='width: 66%;' oninput='iSliderInput(this.value)'> Max");
      // slider functions
      client.println("<script> ");
      client.println("  function iSliderInput(val) {document.getElementById('iSliderValue').innerHTML = val; ");  // place value on client screen in span
      client.println("  $.get('/?ivalue=' + val + '&'); {Connection: close};}");                                  // place value in header to deliver to server
      client.println("</script>"); 
      client.println("<br>");
      client.println("<br>");

      // *************************************************** Switch 1
      // label and span to hold the selected value for display
      //client.println("<h4>Randomize <span id='SW1SliderValue' style='color:black;'></span></h4>");
      // slider
      // pickup current value so that page represents current state when refreshed       
      client.print("Random &nbsp &nbsp &nbsp Off <input id='SW1Slider' type='range' min='0' max='1' step='1' value='");
      client.print(iSW1Value);
      client.println("' style='width: 12%;' oninput='SW1SliderInput(this.value)'> On");
      // slider functions
      client.println("<script> ");
     // client.println("  function SW1SliderInput(val) {document.getElementById('SW1SliderValue').innerHTML = val; ");    // place value on client screen in span
      client.println("  function SW1SliderInput(val) { ");    // place value on client screen in span   
      client.println("  $.get('/?SW1value=' + val + '&'); {Connection: close};}");                                      // place value in header to deliver to server
      client.println("</script>"); 
      client.println("<br>");
      client.println("<br>");
    


      // *************************************************** Switch 2
      // label and span to hold the selected value for display
      //client.println("<h4>Reset <span id='SW2SliderValue' style='color:black;'></span></h4>");
      // slider
      // pickup current value so that page represents current state when refreshed       
      client.print("Fixed Multicolor &nbsp &nbsp &nbsp Off <input id='SW2Slider' type='range' min='0' max='1' step='1' value='");
      client.print(iSW2Value);
      client.println("' style='width: 12%;' oninput='SW2SliderInput(this.value)'> On");
      // slider functions
      client.println("<script> ");
     // client.println("  function SW2SliderInput(val) {document.getElementById('SW2SliderValue').innerHTML = val; ");    // place value on client screen in span
      client.println("  function SW2SliderInput(val) { ");    // place value on client screen in span   
      client.println("  $.get('/?SW2value=' + val + '&'); {Connection: close};}");                                      // place value in header to deliver to server
      client.println("</script>"); 
      client.println("<br>");
      client.println("<br>");
  

      // *************************************************** Switch 3
      // label and span to hold the selected value for display
      //client.println("<h4>Reset <span id='SW2SliderValue' style='color:black;'></span></h4>");
      // slider
      // pickup current value so that page represents current state when refreshed       
      client.print("Reset &nbsp &nbsp &nbsp Off <input id='SW3Slider' type='range' min='0' max='1' step='1' value='");
      client.print(iSW3Value);
      client.println("' style='width: 12%;' oninput='SW3SliderInput(this.value)'> On");
      // slider functions
      client.println("<script> ");
     // client.println("  function SW3SliderInput(val) {document.getElementById('SW3SliderValue').innerHTML = val; ");    // place value on client screen in span
      client.println("  function SW3SliderInput(val) { ");    // place value on client screen in span   
      client.println("  $.get('/?SW3value=' + val + '&'); {Connection: close};}");                                      // place value in header to deliver to server
      client.println("</script>"); 
      client.println("<br>");
      client.println("<br>");
  

      
      
      // close open elements from above
      client.println("</body></html>"); 
  
      // ******************************************* Step 2 -  Handle results - check for return of each possible element
      // display header contents for development / diagnosis
      Serial.println(header);     
      
      // red value returned?
      if(header.indexOf("rvalue=")>=0) 
        {
            leftDelimiter = header.indexOf('=');                                        // left boundary of return value
            rightDelimiter = header.indexOf('&');                                       // right boundary of return value
            sValueString = header.substring(leftDelimiter+1, rightDelimiter);
            iRedValue = sValueString.toInt();
            write_pixels();
            Serial.print("Red value: "); Serial.println(sValueString);                // display "normal" input vaule
        } 
  
      // green value returned?
      if(header.indexOf("gvalue=")>=0) 
        {
            leftDelimiter = header.indexOf('=');                                         // left boundary of return value
            rightDelimiter = header.indexOf('&');                                        // right boundary of return value
            sValueString = header.substring(leftDelimiter+1, rightDelimiter);
            iGreenValue = sValueString.toInt();
            write_pixels();            
            Serial.print("Green value: ");Serial.println(sValueString);                // display "normal" input vaule 
        } 
  
      // blue value returned?
      if(header.indexOf("bvalue=")>=0) 
        {
            leftDelimiter = header.indexOf('=');                                         // left boundary of return value
            rightDelimiter = header.indexOf('&');                                        // right boundary of return value
            sValueString = header.substring(leftDelimiter+1, rightDelimiter);
            iBlueValue = sValueString.toInt();
            write_pixels();            
            Serial.print("Blue value: ");Serial.println(sValueString);                 // display "normal" input vaule
        } 

      // intensity value returned?
      if(header.indexOf("ivalue=")>=0) 
        {
            leftDelimiter = header.indexOf('=');                                         // left boundary of return value
            rightDelimiter = header.indexOf('&');                                        // right boundary of return value
            sValueString = header.substring(leftDelimiter+1, rightDelimiter);
            iIntensityValue = sValueString.toInt();
            write_pixels();            
            Serial.print("Intensity value: ");Serial.println(sValueString);              // display "normal" input vaule
        } 


      // function 1 switch returned?
      if(header.indexOf("SW1value=")>=0) 
        {
            leftDelimiter = header.indexOf('=');                                        // left boundary of return value
            rightDelimiter = header.indexOf('&');                                       // right boundary of return value
            sValueString = header.substring(leftDelimiter+1, rightDelimiter);
            iSW1Value = sValueString.toInt();
            // switch 1 function here...
            if(iSW1Value == 1){
              // if switch is on select a random color for each pixel

// note - this doesn't work well in practice since the rock used in testing (selenite) seems to fuse the colors AND since
// each separate LED gets a random color the net is close to white every time regardless of individual pixels.  find a way to 
// bias all LEDs toward a given random color.  Perhaps get a random value once outside the loop and set all elements to same color?
// individual 'points of color' don't seem to have much practical effect, at least for selenite.

              
              pixels.clear();
              pixels.setBrightness(iIntensityValue);    // apply user-defined intensity instead of random value here...
              for(int i = 0; i<NUMPIXELS; i++)   pixels.setPixelColor(i, pixels.Color(random(0,256), random(0,256), random(0,256)));
              pixels.show();
            }
            else{
              // switch has been moved back to zero, rewrite RGB according to manually set parameters
              write_pixels();  
              // this switch is mutually exclusive with switch 2 - turn off switch 2 if on, this should be reflected in the new page
              iSW2Value = 0;            
            }
            

            Serial.print("Switch1 value: ");Serial.println(sValueString);      
        } 


      // function 2 switch returned?
      if(header.indexOf("SW2value=")>=0) 
        {
            leftDelimiter = header.indexOf('=');                                        // left boundary of return value
            rightDelimiter = header.indexOf('&');                                       // right boundary of return value
            sValueString = header.substring(leftDelimiter+1, rightDelimiter);
            iSW2Value = sValueString.toInt();
            if(iSW2Value == 1){   
              write_fixed_colors();
            }
            else{
              // switch has been moved back to zero, rewrite RGB according to manually set parameters
              write_pixels(); 
              // this switch is mutually exclusive with switch 1 - turn off switch 1 if on, this should be reflected in the new page    
              iSW1Value = 1;     
            }
            
            Serial.print("Switch2 value: ");Serial.println(sValueString);      
        } 

      
      // function 3 switch returned?
      if(header.indexOf("SW3value=")>=0) 
        {
            leftDelimiter = header.indexOf('=');                                        // left boundary of return value
            rightDelimiter = header.indexOf('&');                                       // right boundary of return value
            sValueString = header.substring(leftDelimiter+1, rightDelimiter);
            iSW3Value = sValueString.toInt();
   
            //reboot();
            
            Serial.print("Switch3 value: ");Serial.println(sValueString);      
        } 
        


        
      // ******************************  Step 3  - zero out header and disconnect from server
      header = "";
      client.stop();
      Serial.println("");

    }
}
void write_pixels()
{
  Serial.print("R: ");
  Serial.print(iRedValue);
  Serial.print("  G: ");
  Serial.print(iGreenValue);
  Serial.print("  B: ");
  Serial.print(iBlueValue);
  Serial.print(" I: ");
  Serial.println(iIntensityValue);

  pixels.clear();
  pixels.setBrightness(iIntensityValue);
  for(int i = 0; i<NUMPIXELS; i++)   pixels.setPixelColor(i, pixels.Color(iRedValue, iGreenValue, iBlueValue));
  pixels.show();
}

void write_fixed_colors()
{
  pixels.clear();
  pixels.setBrightness(iIntensityValue);
  pixels.setPixelColor(0, pixels.Color(255, 255, 255));
  pixels.setPixelColor(1, pixels.Color(255, 0, 0));
  pixels.setPixelColor(2, pixels.Color(0, 255, 0));
  pixels.setPixelColor(3, pixels.Color(0, 0, 255));
  pixels.setPixelColor(4, pixels.Color(255, 0, 255));
  pixels.setPixelColor(5, pixels.Color(255, 255, 0));
  pixels.setPixelColor(6, pixels.Color(0, 255, 255));
  pixels.show();
}

 
