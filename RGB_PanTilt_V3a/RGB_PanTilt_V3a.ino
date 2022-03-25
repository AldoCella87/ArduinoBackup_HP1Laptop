// based on framework from Arduino forum: https://forum.arduino.cc/t/esp8266-ios-devices-will-not-connect-help/390880/2
// this version seems to be accessible by IOS devices as well as via laptop using chrome, firefox and Edge
// benefit is there are no external libraries required
// This is too slow, lag between submitting events on client and corresponding action on attached servo...

// 1/26/21 - getting error - "cannonical not found" - fix by:  <link rel="canonical" href="http://www.example.com/">?
// also:  URI Not Found: /connecttest.txt   
//   add <meta name="viewport" content="user-scalable=no"> to the header of web pages that you create, to disable double-tap zooming.

#include <ESP8266WiFi.h> 
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>


// test
#include <Servo.h>
// CONSTANTS
const int panServoPin     = D1;
const int analogOutBlue   = D2;
const int analogOutGreen  = D3;
const int analogOutRed    = D4;  
const int tiltServoPin    = D5;  
const int laserPin        = D6; 

int iRedAnalogValue = 0;
int iPanServoValue = 0;

Servo panServo;
Servo tiltServo;


// end test

 
const char *ssid = "RGB Demo";         
const char *pw = "12345abcde"; 
boolean LEDstate[] = {LOW, false, LOW};

const char* html = 
  "<html>"
    "<head>"
      "<meta charset='utf-8'>"
     // "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
// test
      "<meta name='viewport' content='user-scalable=no, width=device-width, initial-scale=.4'>"

      
      "<title>RGB Blender</title>"
      "<style>"
         ".bt{display:block;width:250px;height:100px;padding:10px;margin:10px;"
        "text-align:center;border-radius:5px;color:grey;font-weight:bold;font-size:70px;text-decoration:none;} "
        "body{background:#666;}"
        ".r{background:#933;}"
        ".g{background:#363;}"
        ".y{background:#EE0;height:100px; width:100px;border-radius:50px;}"
        ".b{background:#000;height:100px;width:100px;border-radius:50px;} "
        ".a{font-size:35px;} td{vertical-align:middle;}"
        
        ".slidecontainer {width: 80%;}"
        ".slider { -webkit-appearance: none;  appearance: none; width: 100%; border-radius: 10px; height: 40px; background: #d3d3d3; outline: none; opacity: 0.7; "
        "-webkit-transition: .2s; transition: opacity .2s; }" 
        ".slider:hover { opacity: 1;  }"
        ".slider::-webkit-slider-thumb {  -webkit-appearance: none; border-radius: 10px; appearance: none; width: 35px; height: 50px; background: #04AA6D; cursor: pointer; }"
      "</style>"
     "</head>"
    "<body>"

       "<table>"
        "<tr>"  
          "<td><div class='TGT0'></div></td>"
          "<td><a class='bt g' href='/L0?v=1'>Increase</a></td>"
          "<td><a class='bt r' href='/L0?v=0'>Decrease</a></td>"
        "</tr>"
        "<tr>"
          "<td><div class='TGT2'></div></td>"
          "<td><a class='bt g' href='/L2?v=1'>Left</a></td>"
          "<td><a class='bt r' href='/L2?v=0'>Right</a></td>"
        "</tr>"
        "<tr>"
          "<td>&nbsp;</td>"
          "<td><a class='bt g a' href='/ALL?v=1'>ALL ON</a></td>"
          "<td><a class='bt r a' href='/ALL?v=0'>ALL OFF</a></td>"
        "</tr>"
      "</table>"
 
      "<br>"

      "<div class='slidecontainer'>"
      //"  <input type='range' min='0' max='180' value='10' class='slider' id='myRange'>"
      "  <input type='range' min='0' max='180' value='10' class='slider' id='myRange' href='/rSlide?v=0'>"

      "  <p>Value: <span id='demo'></span></p>"
      "</div>"
      
      "<script>"
        "var slider = document.getElementById('myRange');"
        //"var output = document.getElementById('demo');"
        "output.innerHTML = slider.value;"
        "slider.oninput = function() {output.innerHTML = this.value;}"
      "</script>"
 
      "<br>"
 
    "</body>"
  "</html>";
 









const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
IPAddress netMsk(255, 255, 255, 0);
DNSServer dnsServer;
ESP8266WebServer server(80);

void setup() {
  pinMode(analogOutRed, OUTPUT);
  pinMode(analogOutBlue, OUTPUT);
  //digitalWrite(analogOutRed, LEDstate[0]);
  //digitalWrite(analogOutBlue, LEDstate[2]);
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(ssid, pw);
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);
  Serial.println("Server up");
  server.on("/", handle_root);
  server.on("/generate_204", handle_root);  // captive portal
  server.on("/L0", handle_L0);
  server.on("/L2", handle_L2);
  server.on("/ALL", handle_ALL);
  // new
  server.on("/rSlide", handle_rSlide);
  
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");


// test
  panServo.attach(panServoPin);
  tiltServo.attach(tiltServoPin);
  panServo.write(90);
  tiltServo.write(90);

// end test

  

}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}

void handleNotFound() {
  Serial.print("\t\t\t\t URI Not Found: ");
  Serial.println(server.uri());
  server.send ( 200, "text/plain", "URI Not Found" );
}

void handle_root() {
  Serial.println("Page served");
  String toSend = html;
  toSend.replace("TGT0", LEDstate[0] ? "y" : "b");
  toSend.replace("TGT2", LEDstate[2] ? "y" : "b");
  server.send(200, "text/html", toSend);
  delay(10);
}



void handle_L0() {

  if (server.hasArg("v")) {
    int state = server.arg("v").toInt() == 1;
    if(state == 1){
        // Increase
         if(iRedAnalogValue < 245){                             // range 0 to 255
            iRedAnalogValue = (iRedAnalogValue + 10);            
            analogWrite(analogOutRed, iRedAnalogValue);
            Serial.print("Analog value is: ");
            Serial.println(iRedAnalogValue);                
         }
    }
    else{
      // decrease
      if(iRedAnalogValue > 10){                                // range 0 to 255
            iRedAnalogValue = (iRedAnalogValue - 10);           
            analogWrite(analogOutRed, iRedAnalogValue);
            Serial.print("Analog value is: ");
            Serial.println(iRedAnalogValue);  
      }
    }
  }
  handle_root();
}

void handle_L2() {
  if (server.hasArg("v")) {                                   // range 0 to 180
    int state = server.arg("v").toInt() == 1;
    if(state == 1){
        // Increase
         if(iPanServoValue < 170){
            iPanServoValue = (iPanServoValue + 10);            
            panServo.write(iPanServoValue);
            Serial.print("Pan Servo value is: ");
            Serial.println(iPanServoValue);                
         }
    }
    else{                                                     // range 0 to 180
      // decrease
      if(iPanServoValue > 10){
            iPanServoValue = (iPanServoValue - 10);           
            panServo.write(iPanServoValue);
            Serial.print("Pan Servo value is: ");
            Serial.println(iPanServoValue);    
      }
    }
  }
  handle_root();
}


void handle_ALL() {
  change_states(0);
  change_states(2);
  handle_root();
}


void handle_rSlide() {
  if (server.hasArg("v")) {
    int state = server.arg("v").toInt() == 0;
    Serial.println("Slider");
  }
}

void change_states(int tgt) {
  if (server.hasArg("v")) {
    int state = server.arg("v").toInt() == 1;
    Serial.print("LED");
    Serial.print(tgt);
    Serial.print("=");
    Serial.println(state);
    LEDstate[tgt] = state ? HIGH : LOW;
    //digitalWrite(tgt, LEDstate[tgt]);
  }
}
