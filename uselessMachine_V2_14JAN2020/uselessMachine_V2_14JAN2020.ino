
// useless machine - a box with a single toggle switch on top.
// When the switch is turned on by the user the box lid opens and an actuator (perhaps a plastic
// hand or arm from a toy doll) emerges which turns the switch to the off position. The actuator returns to the
// home position and the lid closes.
// See: https://www.youtube.com/watch?v=4wWlrBFOdBo for an example.
// The code below assumes two servos however you may simply comment out the lid servo and corresponding delay statements if you go with one servo. 
// Adding two slashes at the start of a line will cause all commands following the slashes on a given line to be considered as a comment
// and thus will not be executed. 

// Here is the starting point code.  Simply open a fresh Arduino sketch, delete all existing template code and paste this entire file (comments and all) into
// the sketch.  Save it with a unique name such as "Useless_V1".  Feel free to save your new versions of this with a different name so you can easily go
// back to a previous version if desired.  Of course, this is just the code to run the motors and read the switch.  You will have to work out the mechanical aspects
// the working of the arm, the placement of the switch, powering the Arduino, the box opening mechanism, etc.  One approach just to get up and running would be to
// simply wire up the motors, switch and LED and test it 'out of the box'.  Each time you turn the switch to the on position the LED should come on and both servos
// should cycle only once.  This would show that the controller and motors are working.  Once you have accomplished this you can build the box, perhaps one
// made of foam core posterboard and duct tape would suffice as a proof of concept. 
// Have fun and Good Luck! 



/***********************************************************************************************
 * Useless machine program for Arduino
 * 2/19/17 - first pass
 * 2/20/17 - corrected comment for resistor value for LED, lengthened actuator distance and 
 * corresponding forward and home wait times.
 * 2/26/17 - added actuator values for my specific implementation (40 and 145)
 * 2/28/17 - further refinements,  added long wait and multiple checks for manual switch deactivation
 * 3/2/17 - added LCD display
 * 3/5/17 - added password mode
 * 3/12/17 - added "speak to me" mode
 * 3/13/17 - added unlock mode parameters and corresponding display, added magnetic sensor
 * 3/14/17 - added light and distance sensors
 * 3/16/17 - added custom random function to replace buggy random();
 * 4/8/17 - refined multi toggle behavior
 * 5/11/17 - added sonic output based on light sensor input
 * 5/13/17 - added accelerometer
 * 5/14/17 - simplified logic in checkLockStatus
 * 5/15/17 - added dispatchBehaviors, removed in-line if statements.  provides for more 'random' behavior
 * 5/17/17 - added display of parameters,  added vacillate and fakeout2 functions, 
 * 5/31/17 - added sound sensor
 ***********************************************************************************************/
 
#include <Servo.h>              // this provides access to the arduino servo functions and is required in order for the code to work

//Uses library from https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads GNU General Public License, version 3 (GPL-3.0) */
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <SparkFun_ADXL345.h>         // SparkFun ADXL345 accelerometer Library

#define MAG_SENSOR_ADDRESS 0x1E //0011110b, I2C 7bit address of HMC5883 magnetic sensor




#define LIGHT_SENSE_PIN  0     // analog input for light sensor
#define RANDOM_SEED_PIN  4     // analog read for random seed
#define MICROPHONE_PIN   1     // microphone analog input
#define ACT_PIN 10             // digital pin 10 goes to the control line of the actuator servo (white (sometimes orange) wire)
#define ALERT_PIN  4           // sonic alert 

// lid and actuator servo positions - adjust these as needed     
// A typical servo motor will only rotate 180 degrees.  When given a command it will rotate to that position and stay there.
// Valid servo commands are between 0 and 180.   

#define ACTUATOR_HOME_POS 140   // the servo starts at this position and returns to this position after turning the switch off
#define ACTUATOR_FWD_POS  40    // the servo goes to this position to push the switch to the off position
#define ACTUATOR_FAKE_POS  ACTUATOR_FWD_POS + 12     // almost touching switch
#define FLAP_POS          132   // the servo goes to this position to push the switch to the off position

                                // servo objects - no need to change these
//Servo lidServo;               // create the lid servo object
Servo actServo;                 // create the actuator servo object

                                // switch and LED pins - no need to change these
#define SWITCH_PIN      2       // assign switch input to digital pin 2.  Switch goes between digital pin 2 and 5V. 
                                // Add a 4700 ohm resistor (yellow, violet, black, brown brown) between this pin and ground to guarantee that the pin
                                // remains in a low state when the switch is opened.
#define LED_PIN         3       // assign LED to digital pin 3.  Anode of LED goes to pin 3. Must use a resistor in series with the LED
                                // Cathode of LED goes to through a 5100 ohm resistor (green, brown, black, brown, brown)to ground.
                                // use a lower value (minimum 200 ohms) for a brighter light
#define US_ECHO_PIN     7       // distance output of ultrasonic sensor
#define US_TRIGGER_PIN  12       // trigger pin for ultrasonic sensor

int bSwitchState = 0;           // variable which holds the current switch reading
bool bFakeout2Played = false;   // prevents "giving up" taunt message when fakeout2 function is executed
int i = 0;                      // generic loop control
int j = 0;
int iMyScore = 0;
int iYourScore = 0;
 

long liresponseTime = 0;        // determines response duration

unsigned int  uitoggleCount = 0;
unsigned long uliNow = 0;                  // holds current time in ms, used for multiple switch toggle count
byte iprevSwitchState = 0;

int itrialCount = 0;            // holds Persistence value, incremented as user progresses
int iMagOrient = 0;             // holds magnetic compass heading
int iLightSense = 0;            // holds ambient light reading
int iMicSense = 0;              // holds ambient noise reading
int iDistance = 0;              // holds ultrasonic distance sensor

int iAccelx, iAccely, iAccelz = 0;  // accelerometer values

bool bTimeFlag1 = false;        // enables special behavior based on time game has been in play
bool bYouWon = false;           // display the "you won" message only once per game
bool bSori = false;             // display the "Sori you lost" message only once per game
int iMaxRandom = 0;             // used to set maximum range for random behavior, higher number = less frequent special behavior   
 
// setup LCD display
#define LCD_ADDRESS1 0x3F       // 2x16 display
#define LCD_ADDRESS2 0x27       // 4x20 display
LiquidCrystal_I2C  lcd(LCD_ADDRESS1,2,1,0,4,5,6,7);  

//setup accelerometer
ADXL345 adxl = ADXL345();             // USE FOR I2C COMMUNICATION


// define behavior parameters

// add light, temperature, humidity, background noise, bluetooth, wifi - some provide a more benevelont behavior
// add "No Cheating" message when switch is turned off manually
// add riddles
// "Are you still there"?
// auto shut off with insult
// if switch is off unit should go back to "your move" without any other messages
// other behaviors:  box tilt, front panel lift, box scoot, wav file ("I'm sorry Dave..."), LCD display "I win again"
// enter Easter Egg mode with x rapid switch transitions,  (perhaps x is a range or it may be displayed as part of the startup screen).  
//   when in easter egg mode allow switch to enter password (cycle through A-Z plus numbers?), pre-display or not the number of digits required for password
//   accept multiple or partial passwords?  use Amazon gift card a prize.
// use photo detector to modulate the personality.. "I like a dark room"
// add reed switch 
// add magnetic orientation
// add accelerometer
// add tilt switch alarm (real or imaginary)
// add password transmission via morse code when in password mode.  Send to AM or FM radio, send via buzzer or LED
// send IR shutoff codes to nearby TVs
// have light level, distance or mag reading control probabilities
// to make more difficult remove names of the measures or change names to meaningless descriptors e.g. "presence"
// once a lock has been unlocked do not display the measurement
// have a higher difficulty mode which requires all locks to be satisfied in a single attempt as opposed to sequentially
// show locks at a designated point in play (e.g. at attempts 50, 100, 150, etc.) for easy mode
// reset timeout timer while in loop counting multiple switch operations.  currently (4/3/17) timer times out if too many multi-switch attempts are made
// use mag sensor reading divisible by x rather than greater than X to allow for any range of values.
// only issue "are you there" message if light level is above or below a certain threshold
// add score reset to zero and score penalty based on capricious infraction
// add tilt alarm, accelerometer and tap detection
// more advanced (more annoying) features should come in after initial play (slowly reveal true personality)
// add passive infrared sensor
// add IR receiver
// add accelerommeter (knock count = flap count)
// first user test - required about 90 minutes given a hint.  Suggestions:  only allow one wisecreack response per type per game, duplicates become old fast.
//   progressively unlock more frustrating behavior.  add noise sensor which integrates average noise level over time.  reveal hints based on environmental conditions
// progressively change behavior over time using millis (max 49 days)
// lock the code to prevent overwrite,  build provision for entering new gift card number, 
// add hint over wifi or bluetooth (have visible connection named "theanswerisfortytwo" or some such.
// add suggestion of wifi or bluetooth connection in speak to me mode
// message "I sense some frustration"
// as conditions become more favorable issue messages "I am feeling more cooperative"
// "I know that you and frank were planning to disconnect me"
// add ticking behavior (using piezo buzzer)
// add simulated gieger counter mode ("taking background reading now...")
// modulate tick frequency based on distance, light reading, compass reading, etc.
// modify iMaxRand variable during play to create more or less special behavior (e.g. tie to light reading)
// set degree of quirkyness based on external factor (e.g. light reading)
// add interaction through serial port, initially print "no hints here", if the right key sequence is entered via serial monitor (42), display a hint
// once answer is found also print to serial port?
// measure individual response time and comment accordingly.  calculate average response time and issue credit or defecit based on user responsiveness after x rounds.
// display simulated myers briggs type index
// comment on favorability or unfavorability of diligence, persistence and time in play as a set of special behaviors

//12/2019 - add recognition of well known sequences (e.g. fibonacci, square of first 3 integers (hint= 2001), etc.)
// add rolling average of response times and boost score for consistently speedy replies.
// add recognition of morse code input
// change LED to RGB and look for differing responses based on color
// add nodemcu web page to show results or provide hints
// add storage in memory of game state
// add bluetooth  node
// find forked version which had voice output to MIT App Inventor app (about December 2018)
// display resposne time via web interface or bluetooth
// allow player to be ahead in the game (see NODEMCU version of SORI)


  
#define  EST_MODE
#ifdef TEST_MODE

  #define THE_ANSWER_TO_LIFE          5           // toggle count required to attempt unlock  
  #define PERSISTENCE_THRESHOLD       0           // minimum number of rounds required to obtain Persistence unlock  
  #define SCORE_THRESHOLD             0           // minimum score required to obtain score unlock  
  #define LIGHT_THRESHOLD             1024        // maximum brightness beyond which unlock is not allowed
  #define MAG_THRESHOLD               0           // minimum reading required to unlock
  #define NOISE_THRESHOLD             5           // minimum reading required to unlock  
  #define TIME_THRESHOLD              0           // minimum minutes of play required to unlock  
  #define HINT_TIME_THRESHOLD         0          // minimum minutes of play required to allow hint to be displayed if called for
  #define DISTANCE_THRESHOLD          16          // maximum reading required to unlock
  #define INACTIVITY_TIME             1200        // seconds to wait before issuing inactivity taunt  
  #define LONG_DELAY_MIN              10          // minimum delay in seconds
  #define LONG_DELAY_MAX              20          // maximum delay in seconds
  #define DISPLAY_READINGS_MIN        0           // minimun number of trials required before sensor readings are displayed
  #define SCORE_BUMP_PROB             0
  #define YOU_WIN_PROB                0
  #define GO_AWAY_PROB                0
  #define SPEAK_TO_ME_MIN             25          // minimum number of trials before speak to me is an option
  #define I_AM_SORI_PROB              0           // "I am Sori ... (pause) "that you are losing"
  #define SCORE_RESET_PROB            0           // "User Response Error Your score has been reset to zero"
  #define SCORE_PENALTY_PROB          0           // "User Response Error Your score has been reset to zero"
  #define GIVEUP_PROB                 0           // probability of displaying wisecrack message regarding persevarance
  #define VOX_RESPONSE_DELAY          4500        // ms to wait for voice response  
  #define LOCK_DISPLAY_TIME           12000       // ms to display dynamic readings during lock check   
  #define SOUND_FEEDBACK_DURATION     50          // used to scale down sensor reading for length of sound display, 8 = reasonable value      
  #define MAX_RAND                    100         // used to set maximum range for random behavior, higher number = less frequent special behavior    
  #define SOUND_SAMPLE_WINDOW         100         // number of ms to accumulate peak to peak sound level
     
  #define LEVEL_TWO                   5           // number of trials requred to increase probability of special behavior
  #define LEVEL_THREE                 15          // number of trials requred to increase probability of special behavior  
  #define LEVEL_FOUR                  25          // number of trials requred to increase probability of special behavior
  #define LEVEL_FIVE                  50          // revert to few special behaviors

  #define DISPLAY_PARAMS              10          // number of consecutive toggles required to display parameters
  #define DISPLAY_HINT                11          // number of consecutive toggles required to display hint
  #define DISPLAY_SENSORS             12          // number of consecutive toggles required to sensor readings
  
  #define MIN_SCORE_ZERO              3           // number of trials before user score begins to accumulate
  #define SHOW_HINT_COUNT             3           // number of trials at which to show the hint

  #define ACCEL_X_THRESHOLD           4           // minimum x acceleration required for unlock
  
#else

  #define THE_ANSWER_TO_LIFE          42          // toggle count required to attempt unlock  
  #define PERSISTENCE_THRESHOLD      130          // miinimum number of rounds required to obtain Persistence unlock  
  #define SCORE_THRESHOLD             100         // minimum score required to obtain score unlock  
  #define LIGHT_THRESHOLD             73          // maximum brightness beyond which unlock is not allowed
  #define MAG_THRESHOLD               0           // minimum reading required to unlock
  #define NOISE_THRESHOLD             10          // minimum reading required to unlock
  #define TIME_THRESHOLD              75          // minimum minutes of play required to unlock  
  #define HINT_TIME_THRESHOLD         30          // minimum minutes of play required to allow hint to be displayed if called for 
  #define DISTANCE_THRESHOLD          16          // maximum reading required to unlock
  #define INACTIVITY_TIME             1200        // seconds to wait before issuing inactivity taunt  
  #define LONG_DELAY_MIN              3           // minimum delay in seconds
  #define LONG_DELAY_MAX              25          // maximum delay in seconds
  #define DISPLAY_READINGS_MIN        50          // minimun number of trials required before sensor readings are displayed
  #define SCORE_BUMP_PROB             1
  #define YOU_WIN_PROB                1
  #define GO_AWAY_PROB                1
  #define SPEAK_TO_ME_MIN             25          // minimum number of trials before speak to me is an option
  #define I_AM_SORI_PROB              1           // "I am Sori ... (pause) "that you are losing"
  #define SCORE_RESET_PROB            1           // "User Response Error Your score has been reset to zero"
  #define SCORE_PENALTY_PROB          1           // penalty for violation of rule 35
  #define GIVEUP_PROB                 1           // probability of displaying wisecrack message regarding persevarance
  #define VOX_RESPONSE_DELAY          4500        // ms to wait for voice response      
  #define LOCK_DISPLAY_TIME           4000        // ms to display dynamic readings during lock check  
  #define SOUND_FEEDBACK_DURATION     40          // used to scale down sensor reading for length of sound display  
  #define MAX_RAND                    100         // used to set maximum range for random behavior, higher number = less frequent special behavior   
  #define SOUND_SAMPLE_WINDOW         100         // number of ms to accumulate peak to peak sound level
  
  #define LEVEL_TWO                   80          // number of trials requred to increase probability of special behavior
  #define LEVEL_THREE                 150         // number of trials requred to increase probability of special behavior  
  #define LEVEL_FOUR                  250         // number of trials requred to increase probability of special behavior
  #define LEVEL_FIVE                  400         // revert to few special behaviors
  
  #define DISPLAY_PARAMS              21          // number of consecutive toggles required to display parameters
  #define DISPLAY_HINT                17          // number of consecutive toggles required to display hint
  #define DISPLAY_SENSORS             29          // number of consecutive toggles required to sensor readings
  #define MIN_SCORE_ZERO              37          // number of trials before user score begins to accumulate
  #define SHOW_HINT_COUNT             155         // number of trials at which to show the hint
  
  #define ACCEL_X_THRESHOLD           4           // minimum x acceleration (g) required for unlock  
#endif


void setup() {
  // mag sensor setup
  Wire.begin();
  
  //Setup HMC5883 magnetic sensor
  Wire.beginTransmission(MAG_SENSOR_ADDRESS); //open communication with HMC5883
  Wire.write(0x02);                           //select mode register
  Wire.write(0x00);                           //continuous measurement mode
  Wire.endTransmission();
  // NOTE:   SDA is Analog4, SCL goes to Analog5
  // end mag sensor setup


  // setup ADXL345 accelerometer
  adxl.powerOn();                     // Power on the ADXL345
  adxl.setRangeSetting(16);           // Give the range settings
                                      // Accepted values are 2g, 4g, 8g or 16g
                                      // Higher Values = Wider Measurement Range
                                      // Lower Values = Greater Sensitivity

  adxl.setSpiBit(0);                  // Configure the device to be in 4 wire SPI mode when set to '0' or 3 wire SPI mode when set to 1
                                      // Default: Set to 1
                                      // SPI pins on the ATMega328: 11, 12 and 13 as reference in SPI Library 
   
  adxl.setActivityXYZ(1, 0, 0);       // Set to activate movement detection in the axes "adxl.setActivityXYZ(X, Y, Z);" (1 == ON, 0 == OFF)
  adxl.setActivityThreshold(75);      // 62.5mg per increment   // Set activity   // Inactivity thresholds (0-255)
 
  adxl.setInactivityXYZ(1, 0, 0);     // Set to detect inactivity in all the axes "adxl.setInactivityXYZ(X, Y, Z);" (1 == ON, 0 == OFF)
  adxl.setInactivityThreshold(75);    // 62.5mg per increment   // Set inactivity // Inactivity thresholds (0-255)
  adxl.setTimeInactivity(10);         // How many seconds of no activity is inactive?

  adxl.setTapDetectionOnXYZ(0, 0, 1); // Detect taps in the directions turned ON "adxl.setTapDetectionOnX(X, Y, Z);" (1 == ON, 0 == OFF)
 
  // Set values for what is considered a TAP and what is a DOUBLE TAP (0-255)
  adxl.setTapThreshold(50);           // 62.5 ms per increment
  adxl.setTapDuration(15);            // 625 μs per increment
  adxl.setDoubleTapLatency(80);       // 1.25 ms per increment
  adxl.setDoubleTapWindow(200);       // 1.25 ms per increment

  // 12/28/19 - accelerometer I2C address apparent conflict exists with compass module.  To overcome this ground SDO pin of accelerometer to 
  // get alternate I2C address
  // end accelerometer setup


  // LCD display setup
  lcd.off(); 
  lcd.begin (16,2);                    
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);
  // end LCD setup
  
  actuatorHome();
  
  displayLCD(F("My name is Sori"),F("Let's play..."), 0,3500);
  displayLCD(F("Score 0 to 0"),F("your move...    "), 0,0);
  
  Serial.begin(9600);                 // start the serial output stream, this enables displaying informational messages on the monitor
  Serial.println("No hints here...");
  
  pinMode(SWITCH_PIN, INPUT);         // configure switch pin as an input.  Connect a resistor between this pin and ground to 
                                      // force it to a low state when switch is open
  pinMode(LED_PIN, OUTPUT);           // configure LED pin as an output, this is for an external LED indicator to accompany the switch
  pinMode(13, OUTPUT);                // configure pin 13 which has an on-board LED, useful for diagnostics
  pinMode(ALERT_PIN, OUTPUT);         // set alert pin to drive sonic alert device
  pinMode(MICROPHONE_PIN, INPUT);
  digitalWrite(ALERT_PIN, 0);         // set alert pin to OFF
  
  liresponseTime = millis()/ (unsigned int) 1000;

  iMaxRandom = MAX_RAND;
  
} // end of setup()


void loop() {

  uitoggleCount = 0;
  bFakeout2Played = false;
  
  lcd.noBlink();
  
  bSwitchState = bcheckSwitchState();                   // check switch state and set LED accordingly

  // if no response from user after a period of time issue an annoyance
  if(millis()/(unsigned int) 1000 - liresponseTime > INACTIVITY_TIME){
    warble(600);
    displayLCD(F("Are you there?  "),F(""), 0,0);
    liresponseTime = millis()/ (unsigned int) 1000;
    }

  if(bSwitchState == 1)                                 // if switch is on then process behaviors
  { 
    iprevSwitchState = 1;            
    uitoggleCount = 1;                                    
    // count number of switch activations
    uliNow = millis();
    while(millis() < uliNow + (unsigned int) 800)
    {
      if(bSwitchState == 1 && iprevSwitchState == 0)
      {
        uitoggleCount++; 
        uliNow = millis();                                   // user is toggling switch, continue to wait for more input     
      }
      iprevSwitchState = bSwitchState;
      bSwitchState = bcheckSwitchState();
    }
  
     /********************************************************
     * 
     * process various behaviors based on toggle count
     * 
     ********************************************************/
     // Check for specific values first for specific responses then check for range of values for generalized response

     //Check 1 - looking for the answer 
    if(uitoggleCount == THE_ANSWER_TO_LIFE) 
    {
      if(iYourScore >= SCORE_THRESHOLD)
      {
         /*************************************
         * check status of all locks.  If all 
         * conditions are met then allow password attempt
         * 
         *************************************/
        if(checkLockStatus())
        {
          if(1) //  add password check, score check or other gate here if desired...
          {
            
            displayLCD(F("Congratulations"),F(""), 0,4000);    // insert name of player if desired     
            displayLCD(F("You win"),F(""), 0,4000);  
            while(bcheckSwitchState())
            {       
              displayLCD(F("Amazon gift"),F("card number is:"), 60,3000);  // TODO - load gift card number
              displayLCD(F("xxxx - 3XPLPM"),F("BZRLE"), 0,7000); 
              displayLCD(F("Turn switch off"),F("to reset game"), 0,3000);    
            }
            // also print gift card number to serial port...
            Serial.println("xxxx - 3XPLPM - BZRLE");                        // TODO - load gift card number  make this a constant
            
            displayLCD(F("Game Over"),F("reset in process"), 0,4000);  
             // reset scores
            swReset();
          }
          else
          { 
            displayLCD(F(" wrong password"),F(""), 60,4000);        
          }
        }
        else
        {
            displayLCD(F("One or more"),F("locks remain"), 0,4000);       
        }
      }
      else
      {
         displayLCD(F("Score too low to"),F("for that..."), 0,4000);   
      }
    }

    // Check 2 - display parameters
    else if (uitoggleCount == DISPLAY_PARAMS)
    {
      if(analogRead(LIGHT_SENSE_PIN) < LIGHT_THRESHOLD)
      {
        displayParams();
      }
      else
      {
        displayLCD(F("Too much light"),F("for that action"), 0,2000);      
      }
    }

    // Check 3 - display hint
    else if (uitoggleCount == DISPLAY_HINT)
    {
      if(analogRead(LIGHT_SENSE_PIN) < LIGHT_THRESHOLD)
      {
        displayLCD(F("Hint must unlock"),F("all locks"), 0,2000);         
        displayLCD(F("... to reveal"),F("prize"), 0,2000);         
        // add other specific hints here if desired
      }
      else
      {
        displayLCD(F("Too much light"),F("for that action"), 0,2000);      
      }
    }

        // Check 4 - display sensors
    else if (uitoggleCount == DISPLAY_SENSORS)
    {
      if(analogRead(LIGHT_SENSE_PIN) < LIGHT_THRESHOLD)
      {
        displaySensors();
      }
      else
      {
        displayLCD(F("Too much light"),F("for that action"), 0,2000);      
      }
    }

    // Check 5 - exhibit behavior showing that the machine is aware of multiple toggles
    // flap door in response to low number of multi toggle events
    // flap door based on number of switch toggles
    else if(uitoggleCount == 4) flap(uitoggleCount);    
    else if(uitoggleCount == 3) flap(uitoggleCount);
    else if(uitoggleCount == 2) flap(uitoggleCount);

    // generalized range checks
    else if (uitoggleCount > 50)
    {
      displayLCD(F("Knock it off!"),F(""), 6000,4000);
    }
    else if (uitoggleCount > 33)
    {
      displayLCD(F("Cut it out!"),F(""), 2000,4000);
    }
    else if (uitoggleCount > 22)
    {
      displayLCD(F("Are you trying"),F("to wear me out?"), 120,2000);
    }
    else if (uitoggleCount > 14)
    {
      displayLCD(F("Are you trying"),F("to annoy me?"), 60,2000);
    }
    else if (uitoggleCount > 8)
    {
      displayLCD(F("Are you trying"),F("to confuse me?"), 0,2000);
    }

    liresponseTime = millis()/ (unsigned int) 1000;                   // get current on time when switch was set
    
    /********************************************************
    * 
    * process some behaviors based on number of trials
    * 
    ********************************************************/
    // show hint only once in the game
    switch(itrialCount)
    {
      case(SHOW_HINT_COUNT):
        {
           displayLCD(F("Hint:    42"),F(""), 0,2000);  //TODO:  remove hardcoded 42 
           Serial.println("42");  
           break;
        }
    }
    
    /********************************************************
    * 
    * process various behaviors based on the total time game 
    * has been in process
    * 
    ********************************************************/
    if(uliCalculateTime()  > HINT_TIME_THRESHOLD && bTimeFlag1 == false)
    {
      // set some special behavior here
      bTimeFlag1 = true;     // do this behavior only once
      displayLCD(F("Hint:    42"),F(""), 100,2000);  
      Serial.println("42");  
    }
    
     /********************************************************
     * 
     * Exhibit one of a number of special behaviors 
     * or no special behavior depending on circumstances
     * 
     ********************************************************/
     dispatchBehaviors();
    

    /*********************************************************
    * 
    * Done with behaviors, turn off switch if it remains on.  
    * calculate and display scores
    * 
    *********************************************************/

    bSwitchState = bcheckSwitchState();  
    if(bSwitchState == 1) 
    {
      // switch remains on, turn it off, calculate and display scores
      turnSwitchOff(); 
      displayScores();   
      itrialCount++;                    // count number of rounds in game                                           
    }
    else
    {
      // switch was manually turned off during processing above
      // inform user to turn switch back on to continue game
      lcd.off();
      actuatorHome();  
      itrialCount++;                    // count number of rounds in game          
      // add one or more conditional taunts here...
      
      if((myrandom(1,iMaxRandom) <= GIVEUP_PROB) || itrialCount < 10) 
      {
        if(!bFakeout2Played)  // do not issue taunt if fakeout2 was played since machine would have turned switch off
        {
        displayLCD(F("Giving up after"),F("only"), 0,0);  
        lcd.setCursor (5,1); 
        lcd.print(itrialCount);   
        lcd.print(F(" tries?"));
        }              
      }
      else
      {
        displayLCD(F("Turn switch on"),F("to continue game"), 0,0);            
      }
    }
    liresponseTime = millis()/ (unsigned int) 1000;                   // reset response time counter to account for time spent playing this round
                                                      // this prevents premature inactivity warning                                                         
  } // end of if(bSwitchState == 1)  
} // end of loop()

/*********************************************************************
 * Check state of toggle switch and set LED accordingly, must have 
 * stable switch reading for x duration
 * 
 *********************************************************************/
bool bcheckSwitchState()
{
  bool bSwitchState;
  bool bPrevSwitchState;
  int i = 0;

  bPrevSwitchState = bSwitchState = digitalRead(SWITCH_PIN);               // read digital input connected to switch 
  
  while(i++ < 6)                                        // must have stable signal for a minimum of time before accepting
  {
    bSwitchState = digitalRead(SWITCH_PIN);             // read digital input connected to switch 
    if (bSwitchState != bPrevSwitchState)
    {
      i = 0;                                            // reset counter, begin new wait 
    }
    delay(8);
    bPrevSwitchState = bSwitchState;
  }
  digitalWrite(LED_PIN, bSwitchState);                  // set LED according to switch reading (switch on turns LED on)
  return bSwitchState;
}

/*********************************************************************
 * deploy actuator slowly to a point almost touching the switch then
 * retract without turning switch off.  
 * 
 *********************************************************************/
void slowDeploy(void)
{
  int i = ACTUATOR_HOME_POS;
  lcd.off();
  actServo.attach(ACT_PIN);    
  while(i-- > ACTUATOR_FWD_POS + 12 && bSwitchState == 1)
  { 
    actServo.write(i);
    delay(60);
    bSwitchState = bcheckSwitchState();
  }
  actServo.detach();    
  delay(myrandom(900,1800));
  actuatorHome();
}

/*********************************************************************
 * display "calculating next move..."  
 * 
 *********************************************************************/
void longWait(void)
{
  displayLCD(F("Calculating next"),F("move..."), 0,0);       
  i = myrandom(LONG_DELAY_MIN,LONG_DELAY_MAX);                                 
  while(j++ < i && bSwitchState == 1){
    delay(700);                  
    lcd.setCursor (8,1);                    
    lcd.print(myrandom(7,9678));
    bSwitchState = bcheckSwitchState();               
  }
  lcd.clear();  
}

/*********************************************************************
 * multi-peek
 * 
 *********************************************************************/
void multiPeek(void)
{
    int iloopMax  = 0;
    delay(myrandom(500, 1800));  
    bSwitchState = bcheckSwitchState();
    i = 0;
    iloopMax = myrandom(2,7);
    while(i++ < iloopMax && bSwitchState == 1)
    {
      peekout(myrandom(90,1200), myrandom(600, 2100));
      bSwitchState = bcheckSwitchState();
    }
}


/*********************************************************************
 * deploy actuator to a point almost touching the switch then retract
 * (does not turn switch off  
 * 
 *********************************************************************/
void fakeout(void)
{
  actuatorForward(ACTUATOR_FAKE_POS);   
  delay(myrandom(900,1800));
  actuatorHome();
}

/*********************************************************************
 * deploy actuator to a point almost touching the switch then wait
 * then turn switch off  
 * 
 *********************************************************************/
void fakeout2(void)
{

  actuatorForward(ACTUATOR_FAKE_POS);   
  delay(myrandom(1500,4500));
  if(bcheckSwitchState()) turnSwitchOff();
  bFakeout2Played = true;

}

/*********************************************************************
 * deploy actuator to a point almost touching the switch then retract
 * then forward, etc... 
 * 
 *********************************************************************/
void vacillate(void)
{
  actuatorForward(ACTUATOR_FAKE_POS);   
  delay(myrandom(900,1800));
  actuatorBack(10);
  delay(myrandom(600,1500));  
  actuatorForward(ACTUATOR_FAKE_POS);   
  delay(myrandom(600,1800));
  actuatorBack(10);
  delay(myrandom(600,1800));
  actuatorHome();
}

/*********************************************************************
 * flap actuator door n times based on iflapCount parameter
 * 
 * 
 *********************************************************************/
void flap(int iflapCount)
{
  int i = 0;
  lcd.off();
  actServo.attach(ACT_PIN);    
  while(i++ < iflapCount && bSwitchState == 1)
  {
    actServo.write(FLAP_POS);
    delay(80);
    actServo.write(ACTUATOR_HOME_POS);
    delay(80);
    bSwitchState = bcheckSwitchState();
  }
  actServo.detach();    
}

/*********************************************************************
 * similar to flap, open actuator door and start actuator according
 * to parameters
 * 
 *********************************************************************/
void peekout(int ipeekTime, int ipeekDelay)
{
  lcd.off();
  actServo.attach(ACT_PIN);    
  actServo.write(FLAP_POS);
  delay(ipeekTime);
  actServo.write(ACTUATOR_HOME_POS);
  delay(600);
  actServo.detach();    
  delay(ipeekDelay);
}  

/*********************************************************************
 * ask user to speak a command then provide a response
 * 
 * 
 *********************************************************************/
void speak(void)
{
  warble(100);
  lcd.clear();                              
  lcd.home();  
  switch (myrandom(1,10))
  {
    case 1:  
      displayLCD(F("Go up one level?"),F("say YES or NO"), 0,0);             
      break;

     case 2:   
      displayLCD(F("Read light lvl?"),F("say YES or NO"), 0,0);             
      break;

    case 3:       
      displayLCD(F("Clearly say the"),F("password"), 0,0);          
      break;
      
   case 4:     
      displayLCD(F("Change mode?"),F("say YES or NO"), 0,0);              
      break;
      
    case 5:       
      displayLCD(F("I am listening.."),F(""), 0,0);   
      break;

    case 6: 
      displayLCD(F("Speak a command"),F("microphone is on"), 0,0);             
      break;
    
    case 7:   
      displayLCD(F("Multiply score?"),F("say YES or NO"), 0,0);           
      break;

    case 8:   
      displayLCD(F("Bluetooth Mode"),F("say ON or OFF"), 0,0);           
      break;    
      
    case 9:   
      displayLCD(F("Activate wifi?"),F("say YES or NO"), 0,0);           
      break;
    }
    
  // give user time to respond to request  
  delay(VOX_RESPONSE_DELAY);

  // continue only if switch is on
  bSwitchState = bcheckSwitchState();
  if(bSwitchState == 1)
  {
    // setup responses to voice input
    switch(myrandom(1,8))
    {
      case 1:
        displayLCD(F("I dont recognize"),F("that command"), 0,0);           
        break;
        
      case 2:
        displayLCD(F("Please try again"),F("later"), 0,0);          
        break;
        
      case 3:
        displayLCD(F("Score too low to"),F("take that action"), 0,0);            
        break;
        
      case 4:
        displayLCD(F("That function is"),F("currently locked"), 0,0);       
        break;
        
      case 5:
        displayLCD(F("Unavailable at"),F("this time"), 0,0);          
        break;              
        
     case 6:
        displayLCD(F("Command not"),F("understood"), 0,0);         
        break;       
        
     case 7:
        if(myrandom(1,101) < 2)
        {
          displayLCD(F("The answer is:"),F("42"), 0,0);              
        }
        else
        {
          displayLCD(F("Can not process"),F("that input"), 0,0);             
        }
        break;          
    }
    // give user time to digest response 
    delay(VOX_RESPONSE_DELAY);
  }
}


/*********************************************************************
 * display all sensor readings
 * 
 * 
 *********************************************************************/
void displaySensors(void)
{

  if(bcheckSwitchState()) displayMagReading();  
  if(bcheckSwitchState()) displayDistanceReading();
  if(bcheckSwitchState()) displayLightReading();
  if(bcheckSwitchState()) displayAccelReading();
  if(bcheckSwitchState()) displayNoiseReading();
  if(bcheckSwitchState()) displayDiligence();
  if(bcheckSwitchState()) displayPersistence();
  if(bcheckSwitchState()) displayTotalTime();
  
}


/*********************************************************************
 * display magnetic sensor reading for a brief period of time
 * 
 * 
 *********************************************************************/
void displayMagReading(void)
{
  lcd.clear();                              
  lcd.home();  
  lcd.print("Orientation is: ");
  uliNow = millis();
  while((millis() < uliNow + (unsigned int) 8000) && (bSwitchState == 1))
  {
    lcd.setCursor (0,1); 
    lcd.print(F("                "));  
    lcd.setCursor (0,1); 
    lcd.print(ireadMagSensor());  
    delay(400);
    bSwitchState = bcheckSwitchState(); 
  }
}

/*********************************************************************
 * display real time distance sensor reading for a brief period of time
 * 
 * 
 *********************************************************************/
void displayDistanceReading(void)
{
  lcd.clear();                              
  lcd.home();  
  lcd.print("Distance is:    ");
  uliNow = millis();
  while((millis() < uliNow + (unsigned int) 6000) && (bSwitchState == 1))
  {
    lcd.setCursor (0,1); 
    lcd.print(F("                "));  
    lcd.setCursor (0,1); 
    lcd.print(getDistance());   
    delay(400);
    bSwitchState = bcheckSwitchState(); 
  }
}

/*********************************************************************
 * display light sensor reading for a brief period of time
 * 
 * 
 *********************************************************************/
void displayLightReading(void)
{
  lcd.clear();                              
  lcd.home();  
  lcd.print("Light reading:  ");
  uliNow = millis();
  while((millis() < uliNow + (unsigned int) 6000) && (bSwitchState == 1))
  {
    lcd.setCursor (0,1); 
    lcd.print(F("                "));  
    lcd.setCursor (0,1); 
    lcd.print(analogRead(LIGHT_SENSE_PIN));   
    delay(400);
    bSwitchState = bcheckSwitchState(); 
  }
}

/*********************************************************************
 * display accelerometer reading for a brief period of time
 * 
 * 
 *********************************************************************/
void displayAccelReading(void)
{
  lcd.clear();                              
  lcd.home();  
  lcd.print("Max acceleration");
  uliNow = millis();
  float Accel_X = 0;
  float MaxAccel_X = 0; 
  while((millis() < uliNow + (unsigned int) 10000) && (bSwitchState == 1))
  {
    Accel_X = getAccelXValue();
    if (Accel_X > MaxAccel_X) MaxAccel_X = Accel_X;
    lcd.setCursor (0,1); 
    lcd.print(F("                "));  
    lcd.setCursor (0,1); 
    lcd.print(MaxAccel_X);

    delay(400);
    bSwitchState = bcheckSwitchState(); 
  }
}

/*********************************************************************
 * display real time sound reading for a brief period of time
 * 
 * 
 *********************************************************************/
void displayNoiseReading(void)
{
  lcd.clear();                              
  lcd.home();  
  lcd.print("Noise reading:  ");
  uliNow = millis();
  while((millis() < uliNow + (unsigned int) 8000) && (bSwitchState == 1))
  {
    lcd.setCursor (0,1); 
    lcd.print(F("                "));  
    lcd.setCursor (0,1); 
    iMicSense = iMeasureSound();
    lcd.print(iMicSense);   
        
    delay(400);
    bSwitchState = bcheckSwitchState(); 
  }
}

/*********************************************************************
 * display Persistence.  This is the total number of rounds which the 
 * player has completed
 * 
 *********************************************************************/
void displayPersistence(void)
{
  lcd.clear();                              
  lcd.home();  
  lcd.print(F("Persevarance is:"));
  lcd.setCursor (0,1);  
  lcd.print(itrialCount);  
  delay(4000);
}

/*********************************************************************
 * display diligence.  This is the average number of responses per  
 * minute over the entire game
 * 
 *********************************************************************/
void displayDiligence(void)
{
  lcd.clear();                              
  lcd.home();  
  lcd.print(F("Diligence Score:"));
  lcd.setCursor (0,1);  
  lcd.print(uliCalculateDiligence());  
  delay(4000);
}

/*********************************************************************
 * display total time in minutes.
 * 
 *********************************************************************/
void displayTotalTime(void)
{
  lcd.clear();                              
  lcd.home();  
  lcd.print(F("Minutes in play:"));
  lcd.setCursor (0,1);  
  lcd.print(uliCalculateTime());  
  delay(4000);
}

/*********************************************************************
 * flicker led, display next move message for random amount of time
 * 
 * 
 *********************************************************************/
void flicker(int iflickerCount)
{
  i = 0;
  j = 0;
  displayLCD(F("Calculating next"),F("move..."), 0,0);       

  while(i++ < iflickerCount && bSwitchState == 1)
  {
    digitalWrite(LED_PIN,1);                            // turn LED off
    delay(myrandom(23,871));
    bSwitchState = bcheckSwitchState();                 // user may have changed switch setting, re-read and act accordingly 
    lcd.setCursor (8,1);                    
    lcd.print(myrandom(2,9897));
    digitalWrite(LED_PIN,0);                            // turn LED off
    delay(myrandom(23,871));
    bSwitchState = bcheckSwitchState();                 // user may have changed switch setting, re-read and act accordingly 
    lcd.setCursor (8,1);                    
    lcd.print(myrandom(2,9897));
  }
}

/*********************************************************************
 * use actuator to turn switch off
 * 
 *********************************************************************/
void turnSwitchOff(void)
{
    actuatorForward(ACTUATOR_FWD_POS);
    actuatorHome();
}

/*********************************************************************
 * move actuator forward to specified position
 * 
 *********************************************************************/
void actuatorForward(int iDistance)
{
  lcd.off();
  actServo.attach(ACT_PIN);           
  actServo.write(iDistance);                        //  move actuator forward to turn switch off
  delay(650);                                       // give the servo time to reach its position 
  actServo.detach();         

  bSwitchState = bcheckSwitchState();               // check switch state and set LED according to switch reading (should be off at this point) 
  lcd.on();
}

/*********************************************************************
 * move actuator back a specific number of steps
 * 
 *********************************************************************/
void actuatorBack(int iSteps)
{
  int iCurrentPos = 0;
  lcd.off();
  actServo.attach(ACT_PIN);  
  // get current servo position  
  iCurrentPos = actServo.read();         
  actServo.write(iCurrentPos + iSteps);             // note - in this application servo low numbers are forward, high numbers are back                  
  delay(650);                                       // give the servo time to reach its position 
  actServo.detach();         
  lcd.on();
}

/*********************************************************************
 * move actuator back to home position
 * 
 * 
 *********************************************************************/
void actuatorHome(void)
// called as part of turnSwitchOff or may be called independently if switch is turned off by user
{
  lcd.off();
  actServo.attach(ACT_PIN);         
  actServo.write(ACTUATOR_HOME_POS);                // move the actuator to home position (back inside the box)
  delay(650);                                       // give the servo time to reach its position
  actServo.detach();      
  lcd.on();
}

/*********************************************************************
 * activate sonic alert briefly to simulate ticking or chirping sound
 * 
 * make ticking sound without
 * turning switch off
 * 10,20,3 gives cricket chirp
 * 2,50,5 buzz
 * 1,10,500 clicks
 * 1,20,1 - 
*********************************************************************/
void tick(int iduration, int icount, int idutycycle)
{
  int i = 0;

  lcd.clear();                              
  lcd.home();  
  lcd.print("WIFI Scanner");   
  lcd.setCursor (0,1);  
  while(i < icount && bSwitchState == 1)
  {
    digitalWrite(ALERT_PIN, 1);         // sonic alert on
    delay(iduration);
    digitalWrite(ALERT_PIN, 0);         // sonic alert off
    delay(iduration * idutycycle); 
    bSwitchState = bcheckSwitchState();
    lcd.setCursor (0,1);  
    lcd.print(i);
    i++;
  }
}

/*********************************************************************
 * activate sonic alert based on sensor reading
 * params:  iduration = duration in seconds
 *********************************************************************/
void soundFeedback(unsigned long iduration)
{
  unsigned long uliend = millis() + (iduration * (unsigned int) 1000);   
  while((millis() < uliend) && (bSwitchState == 1))
  {
    //display ms remaining
    lcd.clear();                              
    lcd.home();  
    lcd.print(uliend - millis());
    // produce sound
    digitalWrite(ALERT_PIN, 1);         // sonic alert on
    delay(1);
    digitalWrite(ALERT_PIN, 0);         // sonic alert off
    delay( analogRead(LIGHT_SENSE_PIN)); 
    bSwitchState = bcheckSwitchState();
  }
  lcd.clear();                              
  lcd.home();  
}

/*********************************************************************
 * turn sonic alert on for duration specified by parameter.  Using
 * sonic alert which has a built in alternating high-low tones.
 * 
 *********************************************************************/
void warble(int iduration)
{
  digitalWrite(ALERT_PIN, 1);         // sonic alert on
  delay(iduration);
  digitalWrite(ALERT_PIN, 0);         // sonic alert off
}

/*********************************************************************
 * calculate score totals for current round and displays
 * 
 * 
 *********************************************************************/
void displayScores()
{
  // calculate scores, of course they are random and the outcome is that the box always wins


  if(itrialCount < MIN_SCORE_ZERO)
  {
    // simple score accumulation for the box, no score accumulation for player
    iMyScore ++;
  }
  else
  {
      // more complex score accumulation
      iMyScore += myrandom(3,6);
      if(myrandom(1,iMaxRandom) <= SCORE_BUMP_PROB)
      {  
        displayLCD(F("Score Multiplier"),F(""), 0,1500);                                
        iYourScore = iMyScore - 1;    
      }
      else
      {
       if(myrandom(1,iMaxRandom) <= SCORE_PENALTY_PROB)   
       {
          displayLCD(F("Score penalty"),F("due to violation"), 0,1500);        
          iYourScore = iYourScore - (iYourScore / 5);
       }
        else
        {
            iYourScore += myrandom(0,3);
        }
      }
  }


  
  // maintain maximum of 4 digits for score
  if(iMyScore > 9999)
  {
     iMyScore = 1;
     iYourScore = 0;
  }
  
  // display results
  if((myrandom(1,iMaxRandom) <= YOU_WIN_PROB) && (bYouWon == false) && (itrialCount > 90))
  {      
    // taunt option 1   
    displayLCD(F("Congratulations"),F("You won!"), 780,2000);   
    displayLCD(F("Just kidding..."),F(""), 0,2000);   
    bYouWon = true;
  }
  // taunt 2
  else if((myrandom(1,iMaxRandom) <= I_AM_SORI_PROB) && (bSori == false) && (itrialCount > 190))
  {      
    displayLCD(F("I am Sori"),F(""), 0,1500);   
    lcd.setCursor (0,1);  
    lcd.print("..that you lost!");
    delay(3000);
    bSori = true;
  }
  if((myrandom(1,iMaxRandom) <= GO_AWAY_PROB) && (itrialCount > 290))
  {                    
    displayLCD(F("I win again"),F("please go away!"), 0,0);   
  }
  else
  {
    // normal behavior          
    lcd.clear();                              
    lcd.home();    
    lcd.print("I win ");
    lcd.setCursor (0,1);  
    lcd.print(iMyScore);
    lcd.print(" to ");  
    lcd.print(iYourScore);   
    lcd.setCursor (0,1);  
    //lcd.print("your move...    ");    
  }
}

/*********************************************************************
 * get magnetic sensor reading
 * 
 * 
 *********************************************************************/
int ireadMagSensor(void)
{
  int x = 0;
  //Tell the HMC5883L where to begin reading data
  Wire.beginTransmission(MAG_SENSOR_ADDRESS);
  Wire.write(0x03); //select register 3, X MSB register
  Wire.endTransmission();
  
  //Read data from each axis, 2 registers per axis
  Wire.requestFrom(MAG_SENSOR_ADDRESS, 6);
  if(6<=Wire.available()){
    x = Wire.read()<<8; //X msb
    x |= Wire.read(); //X lsb
  }
  return x;  
}

/*****************************************************************
 * checkLockStatus
 * Displays status of individual locks, once all locks have been
 * unlocked a password screen may be displayed.
 * 
 * This function may be setup in while loop to continuously 
 * display lock statuses and allow user input to attempt unlock
 * or statuses may be shown once only.  Also may toggle unlock flags
 * for a 'once and done' effect or reset them each time through
 * depending on degree of difficulty desired
 * Option:  Allow user to dwell on each reading and have display show
 * too high - too low (bargraph style with a mid-point target)
 * once unlocked progress to next.
 *****************************************************************/
bool checkLockStatus()
{    
  bool bisUnlocked = false;         // lock status flag

  // Test lock status
  if(checkPersistence())
  {
    if(checkScore())
    {
      if(checkTime())
      {
        if(checkMag())
        {
          if(checkLight())
          {
            if(checkAccel())
            {
              if(checkDistance())
              {
                bisUnlocked = true;
              } 
            }
          }
        }
      }
    }
  }
  
  return(bisUnlocked);
}


bool checkPersistence()
{
  bool bisUnlocked = false;
  /*************************************
  * Persistence Score
  * Note - option to make game more difficult
  * by resetting locks on each attempt.
  *************************************/

  /*************************************
  * Persistence check
  *************************************/      
  lcd.clear();          
  lcd.home (); 
  lcd.print(F("Persistence:   ")); 
  lcd.setCursor (0,1);        
  if(itrialCount >= PERSISTENCE_THRESHOLD)
  {
    bisUnlocked = true;            
    lcd.print(itrialCount);       
    lcd.print(F(" = UNLOCKED")); 
    //warble(100);
  }
  else
  {
    bisUnlocked = false;  
    lcd.print(itrialCount);   
    lcd.print(F(" = LOCKED       ")); 
  }
  delay(3000);
  return(bisUnlocked);
}


bool checkScore()
{
  bool bisUnlocked = false;
  /*************************************
  * Score check
  *************************************/      
  lcd.clear();          
  lcd.home (); 
  lcd.print(F("Score:")); 
  lcd.setCursor (0,1);        
  if(iYourScore >= SCORE_THRESHOLD)
  {
    bisUnlocked = true;   
    lcd.print(iYourScore);       
    lcd.print(F(" = UNLOCKED")); 
    //warble(100);
  }
  else
  {
    bisUnlocked = false;  
    lcd.print(iYourScore);   
    lcd.print(F(" = LOCKED       ")); 
  }
  delay(3000);
  bSwitchState = bcheckSwitchState(); 
  return(bisUnlocked);
}

bool checkTime()
{
  bool bisUnlocked = false;

  /*************************************
  * Time check
  *************************************/      
  uliNow = millis() / (unsigned int) 60000;
  lcd.clear();          
  lcd.home (); 
  lcd.print(F("Time:")); 
  lcd.setCursor (0,1);        
  if(uliNow >= TIME_THRESHOLD)
  {
    bisUnlocked = true;   
    lcd.print(uliNow);       
    lcd.print(F(" = UNLOCKED")); 
    //warble(100);
  }
  else
  {
    bisUnlocked = false;  
    lcd.print(uliNow);   
    lcd.print(F(" = LOCKED       ")); 
  }
  delay(3000);
  bSwitchState = bcheckSwitchState(); 
  return(bisUnlocked);
}



bool checkMag()
{
  bool bisUnlocked = false;

  /*************************************
  * Magnetic Orientation Check
  *************************************/
  lcd.clear();          
  lcd.home (); 
  lcd.print(F("Orientation:    ")); 
  uliNow = millis();
  while((millis() < uliNow + (unsigned int) LOCK_DISPLAY_TIME) && (bSwitchState == 1))
  {
    lcd.setCursor (0,1); 
    lcd.print(F("                ")); 
    lcd.setCursor (0,1);   
    if((iMagOrient = ireadMagSensor()) > 0)
    {
      bisUnlocked = true;   
      lcd.print(iMagOrient);    
      lcd.print(F(" = UNLOCKED")); 
      //warble(100);
    }
    else
    {
       bisUnlocked = false;   
       lcd.print(iMagOrient);       
       lcd.print(F(" = LOCKED       ")); 
    }
    delay(400);
    bSwitchState = bcheckSwitchState(); 
  }
  return(bisUnlocked);
}

bool checkLight()
{
  bool bisUnlocked = false;

  /*************************************
  * Light Level Check
  *************************************/
  lcd.clear();          
  lcd.home (); 
  lcd.print(F("Light level:   ")); 
  uliNow = millis();
  while((millis() < uliNow + (unsigned int) LOCK_DISPLAY_TIME) && (bSwitchState == 1))
  {
    lcd.setCursor (0,1);  
    lcd.print(F("                ")); 
    lcd.setCursor (0,1);  
    iLightSense = analogRead(LIGHT_SENSE_PIN);
    if(iLightSense < LIGHT_THRESHOLD)
    {
      bisUnlocked = true;   
      lcd.print(iLightSense); 
      lcd.print(F(" = UNLOCKED")); 
      //warble(100);   
    }
    else
    {
      bisUnlocked = false;  
      lcd.print(iLightSense); 
      lcd.print(F(" = LOCKED       ")); 
    }
    delay(400);
    bSwitchState = bcheckSwitchState(); 
  }
  return(bisUnlocked);
}

bool checkAccel()
{
  bool bisUnlocked = false;
  /*************************************
  * Accelerometer Reading
  * caution - ensure that a solution exists between accel and mag sensor readings
  * perhaps use tap or double tap rather than tilt orientation to avoid creating an intractible problem
  *************************************/
  float fMaxAccel = 0.0;
  float X_Accel = 0.0;
  int ADXL345 = 0x53; // The ADXL345 sensor I2C address
  
  lcd.clear();          
  lcd.home (); 
  lcd.print(F("Max acceleration")); 
  uliNow = millis();
  while((millis() < uliNow + (unsigned int) LOCK_DISPLAY_TIME) && (bSwitchState == 1))
  {
    lcd.setCursor (0,1);
    lcd.print(F("                ")); 
    lcd.setCursor (0,1);  
    X_Accel = getAccelXValue();
    if (X_Accel > fMaxAccel) fMaxAccel = X_Accel;
    if(fMaxAccel > ACCEL_X_THRESHOLD )   
    {
      bisUnlocked = true;       
      lcd.print(fMaxAccel); 
      lcd.print(F(" = UNLOCKED")); 
      //warble(100);   
    }
    else
    {
      bisUnlocked = false;  
      lcd.print(fMaxAccel); 
      lcd.print(F(" = LOCKED       ")); 
    }
    delay(400);
    bSwitchState = bcheckSwitchState(); 
  }
  return(bisUnlocked);
}

bool checkNoise()
{
  bool bisUnlocked = false;

  /*************************************
   * Noise Reading
   * 
   *************************************/
 
  lcd.clear();          
  lcd.home (); 
  lcd.print(F("Noise level:   ")); 
  
  uliNow = millis();
  while((millis() < uliNow + (unsigned int) LOCK_DISPLAY_TIME) && (bSwitchState == 1))
  {
    lcd.setCursor (0,1);  
    lcd.print(F("                ")); 
    lcd.setCursor (0,1);  
    iMicSense = iMeasureSound();    
    if(iMicSense < NOISE_THRESHOLD)
    {
      bisUnlocked = true;   
      lcd.print(iMicSense); 
      lcd.print(F(" = UNLOCKED")); 
      //warble(100);   
    }
    else
    {
      bisUnlocked = false;  
      lcd.print(iMicSense); 
      lcd.print(F(" = LOCKED       ")); 
    }
    delay(400);
    bSwitchState = bcheckSwitchState(); 
  }
 
  return(bisUnlocked);
}

bool checkDistance()
{
  bool bisUnlocked = false;   
  /*************************************
  * Distance Check
  * 
  *************************************/
  lcd.clear();          
  lcd.home (); 
  lcd.print(F("Distance:       ")); 
  uliNow = millis();
  while((millis() < uliNow + (unsigned int) LOCK_DISPLAY_TIME) && (bSwitchState == 1))
  {
    lcd.setCursor (0,1);  
    lcd.print(F("                ")); 
    lcd.setCursor (0,1);  
    iDistance = getDistance();   
    if(iDistance < DISTANCE_THRESHOLD)
    {
      bisUnlocked = true;   
      lcd.print(iDistance); 
      lcd.print(F(" = UNLOCKED")); 
      //warble(100);   
    }
    else
    {
      bisUnlocked = false;   
      lcd.print(iDistance); 
      lcd.print(F(" = LOCKED       ")); 
    }
    delay(400);
    bSwitchState = bcheckSwitchState(); 
  }
  return(bisUnlocked);
}


/*********************************************************************
 * get distance sensor reading
 * 
 * 
 *********************************************************************/
int getDistance()
{
  // adapted from: https://gist.github.com/flakas/3294829
  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(US_TRIGGER_PIN, OUTPUT);
  digitalWrite(US_TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(US_TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(US_TRIGGER_PIN, LOW);

  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(US_ECHO_PIN, INPUT);
  return (pulseIn(US_ECHO_PIN, HIGH) / 72 / 2);
}


/****************************************************
 * Calculate diligence = responses per minute
 * over entire game
 ***************************************************/
unsigned long uliCalculateDiligence()
{
  return (itrialCount / uliCalculateTime()); // responses per minute
}

/****************************************************
 * Calculate total time in play
 *  this is correct only if the game is powered on 
 *  at the same time that play starts.  If game is reset this will be incorrect
 *  TODO - fix this
 ***************************************************/
unsigned long uliCalculateTime()
{
  return ((millis()/ (unsigned int) 60000) );
}

/*********************************************************************
 * function to deliver pseudo-random number
 * 
 * 
 *********************************************************************/
int myrandom(int min, int max)
{
  randomSeed(analogRead(RANDOM_SEED_PIN));
  int iret = 0;
  //iret = min + rand()% (max - min); 
  // replacing with call to micros() will only seem random because of difference in timing of user input.
  iret = min + micros()% (max - min); 
  
  #define xAND_TEST_MODE
  #ifdef RAND_TEST_MODE
      Serial.println();
      Serial.print(min);
      Serial.print(", ");
      Serial.print(max);
      Serial.println();
      for (int i = 0; i<100; i++)
      {
        iret = min + rand() % (max - min);
        Serial.println(iret);
      }
  #endif

  #define xISP_RAND
  #ifdef DISP_RAND
      Serial.println();
      Serial.print(min);
      Serial.print(", ");
      Serial.print(max);
      Serial.println();
      Serial.println(iret);
  #endif
  return iret;
}

/*********************************************************************
 * general purpose display manager with optional tone alert and delay
 * 
 *********************************************************************/
void displayLCD(String sline1, String sline2, int itoneLen, int idelay)
{
    lcd.off(); 
    lcd.clear();          
    lcd.home (); 
    lcd.print(sline1); 
    lcd.setCursor (0,1);        
    lcd.print(sline2); 
    lcd.on();  
    if (itoneLen > 0) warble(itoneLen);
    if (idelay > 0) delay(idelay);
}

/*********************************************************************
 * reset score and counters after user wins the game
 * 
 *********************************************************************/
void swReset() // Restarts program from beginning but does not reset the peripherals and registers
{
  //http://forum.arduino.cc/index.php?topic=49581.0
  // this appears to reset program variables (e.g. Persistence) so is useful to reset locks after
  // player wins.
  asm volatile ("  jmp 0");  
} 

/*********************************************************************
 * randomly dispatch one behavior, some behaviors have prerequisites
 * 
 *********************************************************************/
void dispatchBehaviors(void)
{
  #define NUM_CASES 19
  int ibehaviorModFactor;               //establishes width of band holding normal behaviors.  the smaller the number the greater 
                                        // the probability of special behavior being exhibited
  // modulate behavior based on Persistence.  the longer the player plays, the more special behaviors are expressed
  if(itrialCount > LEVEL_FOUR) ibehaviorModFactor = 2;
  else if (itrialCount > LEVEL_THREE) ibehaviorModFactor = 5;
  else if (itrialCount > LEVEL_TWO) ibehaviorModFactor = 7; 
  else if(itrialCount > LEVEL_FIVE) ibehaviorModFactor = 10;        // at some point revert back to initial (sparse) special behaviors
  else ibehaviorModFactor = 10;                                     // starting point special behavior probability (approx one in 10)




  
 switch(myrandom(1,(NUM_CASES * ibehaviorModFactor)))  
  //switch(myrandom(1,(analogRead(LIGHT_SENSE_PIN)+ NUM_CASES + 1))) // have probability of special behavior be related to light level, ensure that minimum of 'n' behaviors available                                                 
  {
    case 1:
      longWait();
      break;

   case 2:
      slowDeploy();
      break;

    case 3:
      fakeout();
      break;

    case 4:
      peekout(myrandom(600,1200), myrandom(900, 1700));
      break;
    
    case 5: 
      tick(1,(analogRead(LIGHT_SENSE_PIN) / 5),90);
      break;
      
    case 6:
      soundFeedback(analogRead(MICROPHONE_PIN)/SOUND_FEEDBACK_DURATION);
      break;
  
    case 7:
      flicker(myrandom(3,27));
      break;

    case 8:
      multiPeek();
      break;

    case 9:
      if(itrialCount > SPEAK_TO_ME_MIN) speak();
      break;
  
    case 10:
      if(itrialCount > DISPLAY_READINGS_MIN + 0) displayMagReading();
      break;

    case 11:
      if(itrialCount > DISPLAY_READINGS_MIN + 10) displayLightReading();
      break;
      
    case 12:
      if(itrialCount > DISPLAY_READINGS_MIN + 20) displayAccelReading();
      break;
  
    case 13:
      if(itrialCount > DISPLAY_READINGS_MIN + 30) displayNoiseReading();
      break;

    case 14:
      if(itrialCount > DISPLAY_READINGS_MIN + 50) displayDistanceReading();
      break;

    case 15:  
      if(itrialCount > DISPLAY_READINGS_MIN + 60) displayPersistence();
      break;
      
    case 16:
      flap(myrandom(2,13));
      break; 
           
    case 17:
      if(itrialCount > DISPLAY_READINGS_MIN + 70) displayParams();
      break;

    case 18:
      vacillate();
      break;

    case 19:
      fakeout2();
      break;
      
    default:
       // no special actions in this case.  This should be the case a substantial portion of the time
       // therefore ensure that the random range for the switch is always greater than the maximum
       // number of cases above
       break;
  }  // end switch
} //end dispatchBehaviors


/***************************************************
 * 
 * measure sound
 * returns:  integer sound level
 **************************************************/
// adapted from: 
// https://learn.adafruit.com/adafruit-microphone-amplifier-breakout/measuring-sound-levels
int iMeasureSound()
{
   unsigned long startMillis= millis();  // Start of sample window
   unsigned int peakToPeak = 0;   // peak-to-peak level
   unsigned int signalMax = 0;
   unsigned int signalMin = 1024;
   unsigned int sample;
   
   while (millis() - startMillis < SOUND_SAMPLE_WINDOW)
   {
      sample = analogRead(MICROPHONE_PIN);
      if (sample < 1024)  // toss out spurious readings
      {
         if (sample > signalMax)
         {
            signalMax = sample;  // save just the max levels
         }
         else if (sample < signalMin)
         {
            signalMin = sample;  // save just the min levels
         }
      }
   }
   peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
   return(peakToPeak);
}

/***************************************************
 * 
 * display parameters
 * 
 **************************************************/
void displayParams(void)
{
  int iTemp = 0;
  if(bcheckSwitchState())
  {  
    lcd.clear();          
    lcd.home (); 
    iTemp = THE_ANSWER_TO_LIFE;
    lcd.print(F("Answer to Life"));  
    lcd.setCursor (0,1);  
    lcd.print(iTemp);
    delay(2000);
  }

  if(bcheckSwitchState())
  {  
    lcd.clear();          
    lcd.home (); 
    iTemp = PERSISTENCE_THRESHOLD;
    lcd.print(F("Min Persistence"));  
    lcd.setCursor (0,1);  
    lcd.print(iTemp);
    delay(2000);
  }

  if(bcheckSwitchState())
  {  
    lcd.clear();          
    lcd.home (); 
    iTemp = SCORE_THRESHOLD;
    lcd.print(F("Min Score"));  
    lcd.setCursor (0,1);  
    lcd.print(iTemp);
    delay(2000);
  }

  if(bcheckSwitchState())
  {  
    lcd.clear();          
    lcd.home (); 
    iTemp = LIGHT_THRESHOLD;
    lcd.print(F("Max Light"));  
    lcd.setCursor (0,1);  
    lcd.print(iTemp);
    delay(2000);
  }

  if(bcheckSwitchState())
  {  
    lcd.clear();          
    lcd.home (); 
    iTemp = MAG_THRESHOLD;
    lcd.print(F("Min Mag Reading"));  
    lcd.setCursor (0,1);  
    lcd.print(iTemp);
    delay(2000);
  }

  if(bcheckSwitchState())
  {  
    lcd.clear();          
    lcd.home (); 
    iTemp = NOISE_THRESHOLD;
    lcd.print(F("Min Noise"));  
    lcd.setCursor (0,1);  
    lcd.print(iTemp);
    delay(2000);
  }

  if(bcheckSwitchState())
  {  
    lcd.clear();          
    lcd.home (); 
    iTemp = TIME_THRESHOLD;
    lcd.print(F("Min Time"));  
    lcd.setCursor (0,1);  
    lcd.print(iTemp);
    delay(2000);
  }
  
  if(bcheckSwitchState())
  {  
    lcd.clear();          
    lcd.home (); 
    iTemp = DISTANCE_THRESHOLD;
    lcd.print(F("Min Distance"));  
    lcd.setCursor (0,1);  
    lcd.print(iTemp);
    delay(2000);
  }

  if(bcheckSwitchState())
  {  
    lcd.clear();          
    lcd.home (); 
    iTemp = DISPLAY_PARAMS;
    lcd.print(F("Display Params"));  
    lcd.setCursor (0,1);  
    lcd.print(iTemp);
    delay(2000);
  }

  if(bcheckSwitchState())
  {  
    lcd.clear();          
    lcd.home (); 
    iTemp = DISPLAY_HINT;
    lcd.print(F("Display Hint"));  
    lcd.setCursor (0,1);  
    lcd.print(iTemp);
    delay(2000);
  }

  if(bcheckSwitchState())
  {  
    lcd.clear();          
    lcd.home (); 
    iTemp = DISPLAY_SENSORS;
    lcd.print(F("Display Sensors"));  
    lcd.setCursor (0,1);  
    lcd.print(iTemp);
    delay(2000);
  }    
  
}

float getAccelXValue(void)
{

      float X_out, Y_out, Z_out;  // Outputs
      int ADXL345 = 0x53; // The ADXL345 sensor I2C address
      Wire.beginTransmission(ADXL345);
      Wire.write(0x32); // Start with register 0x32 (ACCEL_XOUT_H)
      Wire.endTransmission(false);
      Wire.requestFrom(ADXL345, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
      X_out = ( Wire.read()| Wire.read() << 8); // X-axis value
      //X_out = X_out/256; //For a range of +-2g, we need to divide the raw values by 256, according to the datasheet
      Y_out = ( Wire.read()| Wire.read() << 8); // Y-axis value
      //Y_out = Y_out/256;
      Z_out = ( Wire.read()| Wire.read() << 8); // Z-axis value
      //Z_out = Z_out/256;
    
      //Serial.print("Xa= ");
      Serial.print(X_out);
      //Serial.print("   Ya= ");
      Serial.print(",");
      Serial.print(Y_out);
      //Serial.print("   Za= ");
      Serial.print(",");
      Serial.println(Z_out);
 
      return(X_out);
  
}
