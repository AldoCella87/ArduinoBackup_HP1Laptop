// 10/28/21 - removed deep sleep test code, adjusted for use on Wemos D1 board for use in St. Augustine
// 9/18/21 - next gen - using low cost outdoor LED light (Patriot 343-4442 from Menards). changed message to be a single string instead of an array of characters.  
// changed send message operation to ignore null character at end of new message string.    
// added sleep functionality based on info at: https://www.electroniclinic.com/arduino-sleep-modes-automatic-and-manual-to-save-power-arduino-deep-sleep/

// 9/13/21 - corrected spelling of "remembrance"
// 8/24/21 - unit placed in Corpus Christi Cemetery, Chambersburg PA (used 3M auto trim tape, Asked Nate to caulk top edge)
//           consider enhancements:  only use 1 mounting point or slots for circuit board to prevent thermal cycle damage to board,
//                                   provide some way of pressure equalization as box heats and cools,
//                                   lock latches to discourage tampering, seal gasket with thin coating of grease (pressure
//                                   differences may cause gasket to leak)

// 8/19/21 AnnaMarie corrected Al's spelling of "Nona=Nonna", refined message
// 8/15/21 - added multiple messages, added gift card message.  plan to install at family gravesite in PA
// 7/16/21 - corrected delay time (was int/ should be long.  added all letters and numbers, changed error handling, add amazon gift card 
// 1/17/17 - Louie - here is the code which is loaded on the Arduino which I sent you.  -Al
// created message matrix 12/28/15 AE
// 1/17/17 activated random behavior, added separate LED output  AE


//#include <LowPower.h>
int tonePin = 3;
int LEDPin = 0;     // pin 0 is digital 3 on Wemos board
                    // do not use digital 2 as this is the built in LED and it is inverted

//int wakeUpPin = 3;    // needs to be specified to enter sleep mode however we will not use an interrupt to wake up processor
                      // processor will only operate one time per day (per power cycle) (recommended to be pins 2 or 3 for Nano)


// $50 Amazon certificate 8/16/21
char sMessage[] =   "Please stand by for a message   Please stand by for a message   Please stand by for a message  message follows "
                    "Hello friends  Have a beer and a slice of pizza in memory of good old Papa Woody.  Remember to love each other and to treat each "
                    "other with kindness and respect.  "
                    "Use this code at Jeffs Place.  The code is only valid for one use and is offered on a first come first served basis "
                    "  The code is AQKK WRCZBK 5MGA3  "  
                    "  Repeat  AQKK WRCZBK 5MGA3  "  
                    "Papa Woody says Neeeever Better.  Ciao for now   end of message";


int freq;
int dotPeriod;
int dahPeriod;
int relaxtime;
int letterSpace;
int wordSpace;  
unsigned long delayTime;    // max value 4 billion and some...

void setup() {

  pinMode(LEDPin, OUTPUT);
  digitalWrite(LEDPin, LOW);

  digitalWrite(LED_BUILTIN, HIGH);    // high turns off built in blue LED on Wemos D1 mini (this is on digital 2)
  
  pinMode(tonePin, OUTPUT);
  Serial.begin(9600);
  delay(5000);
}

void loop() {
  int i;
  randomSeed(analogRead(0));
  
  // establish random behavior
  // 1 hr is 3,600,000 ms
  // 2 hrs is 7,200,000 ms
  // 4 hrs is 14,400,000 ms
  // 10 min is 600,000 ms

#define EST_MODE  
#ifdef TEST_MODE
  freq = 1000;
  dotPeriod = 160;
  delayTime = 300000;  // 300000 = 5 mins  
#else 
  freq = random(600,1100);
  dotPeriod = 160;  
  //delayTime = random(14400000,18000000);  //delay between 4 and 5 hours
  //delayTime = random(240000,360000);  //delay between 4 and 6 minutes
  delayTime = 30000;     
#endif

  // establish other code timings based on dot period
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
  for(i=0;i<sizeof(sMessage)-1;i++)
  {
    playLetter(sMessage[i]);
    Serial.write(sMessage[i]);
    if((i+1)%60 == 0) Serial.println();
    delay(letterSpace);
  }


  delay(delayTime);
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
