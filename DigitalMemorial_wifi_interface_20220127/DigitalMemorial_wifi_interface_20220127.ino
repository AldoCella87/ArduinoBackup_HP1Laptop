// This is a fusion of St. Augustine Digital Memorial and RGB Wifi control
// the purpose is to provide web interface to existing digital memorial applicaiton.  
// this is experimental... architecture may need revision...

// TODO
// allow server to service requests will sending message?  yield()?
// remove all 'delays' in message sending functions and replace with check based on elapsed time

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


// DM: 

int tonePin = 3;
int LEDPin = 0;     // pin 0 is digital 3 on Wemos board
                    // do not use digital 2 as this is the built in LED and it is inverted

// $50 Amazon certificate 8/16/21
char sMessage[] =   "Testing";


int freq;
int dotPeriod;
int dahPeriod;
int relaxtime;
int letterSpace;
int wordSpace;  
unsigned long delayTime;    // delay in ms,  max value 4 billion and some...
unsigned long timestamp;    // holds time of last transmission




// end DM                    


// CONSTANTS

 

 
const char *ssid = "DigitalMemorial";         
const char *pw = "12345abcde"; 
 

const char* html = 
  "<html>"
    "<head>"
      "<meta charset='utf-8'>"
     // "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
// test
      "<meta name='viewport' content='user-scalable=no, width=device-width, initial-scale=.4'>"

      
      "<title>Digital Memorial</title>"
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
          "<td><div class='TGT0'>'Period:  '</div></td>"
          "<td><a class='bt g' href='/L0?v=1'>Increase</a></td>"
          "<td><a class='bt r' href='/L0?v=0'>Decrease</a></td>"
        "</tr>"
        "<tr>"
          "<td><div class='TGT2'>Freq: </div></td>"
          "<td><a class='bt g' href='/L2?v=1'>Increase</a></td>"
          "<td><a class='bt r' href='/L2?v=0'>Decrease</a></td>"
        "</tr>"
        "<tr>"
         "<td><div class='TGT2'>Run: </div></td>"
          "<td><a class='bt g a' href='/ALL?v=1'>ON</a></td>"
          "<td><a class='bt r a' href='/ALL?v=0'>OFF</a></td>"
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


// digital memorial stuff


  pinMode(LEDPin, OUTPUT);
  digitalWrite(LEDPin, LOW);

  digitalWrite(LED_BUILTIN, HIGH);    // high turns off built in blue LED on Wemos D1 mini (this is on digital 2)
  
  pinMode(tonePin, OUTPUT);
  //Serial.begin(9600);
  delay(5000);

// assign initial values - check for test mode
#define EST_MODE  
#ifdef TEST_MODE
  freq = 1000;
  dotPeriod = 160;
  delayTime = 300000;  // 300000 = 5 mins  
#else 
  //freq = random(600,1100);
  freq = 1000;
  dotPeriod = 160;  
  //delayTime = random(14400000,18000000);  //delay between 4 and 5 hours
  //delayTime = random(240000,360000);  //delay between 4 and 6 minutes
  delayTime = 30000;     
#endif

  

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
  //toSend.replace("TGT0", LEDstate[0] ? "y" : "b");
  //toSend.replace("TGT2", LEDstate[2] ? "y" : "b");
  server.send(200, "text/html", toSend);
  delay(10);
  send_message();
}



void handle_L0() {

  if (server.hasArg("v")) {
    int state = server.arg("v").toInt() == 1;
    if(state == 1){
      // Increase value (sends message code more slowly)
      if(dotPeriod < 200) dotPeriod += 10;
    }
    else{
      // decrease value (sends message more quickly)
      if(dotPeriod > 50) dotPeriod -= 10;
    }
    Serial.print("dotPeriod is: ");
    Serial.println(dotPeriod);   
  }
  handle_root();
}

void handle_L2() {

  if (server.hasArg("v")) {
    int state = server.arg("v").toInt() == 1;
    if(state == 1){
      // Increase value  
      if(freq < 1975) freq += 25;
    }
    else{
      // decrease value  
      if(freq > 525) freq -= 25;
    }
    Serial.print("freq is: ");
    Serial.println(freq);   
  }
  handle_root();
}



void handle_ALL() {



  handle_root();
}


void handle_rSlide() {
  if (server.hasArg("v")) {
    int state = server.arg("v").toInt() == 0;
    Serial.println("Slider");
  }
}





void send_message() {

  // establish other code timings based on dot period which is configurable
  dahPeriod = (dotPeriod *3);
  relaxtime = (dotPeriod);
  letterSpace = (dotPeriod *3);   // standard letter space is 3x dot length  
  wordSpace = (dotPeriod *7);     // standard word space is 7x dot length 

  // display results of random variable assignments
  Serial.println();
  Serial.println("Digital Memorial  10/29/21");
  Serial.println();
  Serial.print("frequency:  ");
  Serial.println(freq);
  Serial.print("dotPeriod:  ");
  Serial.println(dotPeriod);
  Serial.print("dahPeriod:  ");
  Serial.println(dahPeriod);
  Serial.print("delayTime:  "); 
  Serial.println(delayTime);

  // send message
  for(int i=0;i<sizeof(sMessage)-1;i++)
  {
    playLetter(sMessage[i]);
    Serial.write(sMessage[i]);
    if((i+1)%60 == 0) Serial.println();
    delay(letterSpace);
  }


  //delay(delayTime);
}



void playLetter(char x)
  {
    // change order to put frequently used characters at top of switch if desired.
    switch (x){
      case 'A':
      case 'a':
        di();dah(); return;
      case 'B':
      case 'b':
        dah();di();di();dit(); return;
      case 'C':
      case 'c':
        dah();di();dah();dit();return;    
      case 'D':
      case 'd':
        dah();di();dit(); return;      
      case 'E':
      case 'e':
        dit(); return;
      case 'F':
      case 'f':
        di();di();dah();dit(); return;
      case 'G':
      case 'g':
        dah();dah();dit(); return;        
      case 'H':
      case 'h':
        di();di();di();dit(); return;
      case 'I':
      case 'i':
        di();dit(); return;
      case 'J':
      case 'j':
        di();dah();dah();dah(); return;
      case 'K':
      case 'k':
        dah();di();dah(); return;
      case 'L':
      case 'l':
        di();dah();di();dit(); return;
      case 'M':
      case 'm':
        dah();dah(); return;
      case 'N':
      case 'n':
        dah();dit(); return;
      case 'O':
      case 'o':
        dah();dah();dah(); return;
      case 'P':
      case 'p':
        di();dah();dah();dit(); return;     
      case 'Q':
      case 'q':
        dah();dah();di();dah(); return;
      case 'R':
      case 'r':
        di();dah();dit(); return;
      case 'S':
      case 's':
        di();di();dit(); return;
      case 'T':
      case 't':
        dah(); return;
      case 'U':
      case 'u':
        di();di();dah(); return;
      case 'V':
      case 'v':
        di();di();di();dah(); return;                
      case 'W':
      case 'w':
        di();dah();dah(); return;
      case 'X':
      case 'x':
        dah();di();di();dah(); return;
      case 'Y':
      case 'y':
        dah();di();dah();dah(); return;        
      case 'Z':
      case 'z':
        dah();dah();di();dit(); return;
      case '-':
        dah();di();di();di();dah(); return; 
      case '_':
        dahhh(); return; 
      case '>':
        dah();dahh();di();dit(); return;  //custom Z for the Zipper        
      case '!': // stands in for SK (end of transmission / Silent Key)  with special emphasis on last dah
        di();di();dit();dah();di();FinalDah(); return;
      case '.':
        di();dah();di();dah();di();dah(); return;
      case '1':
        di();dah();dah();dah();dah(); return;
      case '2':
        di();di();dah();dah();dah(); return;
      case '3':
        di();di();di();dah();dah(); return; 
      case '4':
        di();di();di();di();dah(); return; 
      case '5':
        di();di();di();di();dit(); return; 
      case '6':
        dah();di();di();di();dit(); return;   
      case '7':
        dah();dah();di();di();dit(); return;
      case '8':
        dah();dah();dah();di();dit(); return;
      case '9':
        dah();dah();dah();dah();dit(); return;
      case '0':
        dah();dah();dah();dah();dah(); return;        
      case ' ':
        delay(wordSpace); return;
      default:
        // ignore unrecognized characters  
        // ignore non-recognized characters, this allows <CR> and <LF> in message without effect
        return;  
    }
  }
   
void ErrInd()
{
  tone(tonePin, freq / 10);
  delay(dahPeriod);
  noTone(tonePin);
  delay(relaxtime);
}

void dit()
{
  digitalWrite(LEDPin, HIGH);
  tone(tonePin, freq);
  delay(dotPeriod);
  digitalWrite(LEDPin, LOW);
  noTone(tonePin);
  delay(relaxtime);
}

void dah()
{
  digitalWrite(LEDPin, HIGH);
  tone(tonePin, freq);
  delay(dahPeriod);
  digitalWrite(LEDPin, LOW);
  noTone(tonePin);
  delay(relaxtime);
}

void dahh()
{
  digitalWrite(LEDPin, HIGH);
  tone(tonePin, freq);
  delay(dahPeriod);
  delay(dahPeriod);  //extra long dah for call sign, characteristic of the Zipper
  digitalWrite(LEDPin, LOW);
  noTone(tonePin);
  delay(relaxtime);
}

void dahhh()
{
  digitalWrite(LEDPin, HIGH);
  tone(tonePin, freq);
  delay(dahPeriod*6);
  digitalWrite(LEDPin, LOW);
  noTone(tonePin);
  delay(relaxtime);
}
  
void FinalDah()
{
  digitalWrite(LEDPin, HIGH);
  tone(tonePin, freq);
  delay(dahPeriod * 3);
  digitalWrite(LEDPin, LOW);
  noTone(tonePin);
  delay(relaxtime);
}

void di()
{
  dit();
}
