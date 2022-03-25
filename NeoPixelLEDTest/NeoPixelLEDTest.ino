/*
NeoPixel LEDs

modified on 7 May 2019
by Saeed Hosseini @ Electropeak
**This code is based on Adafruit NeoPixel library Example**
https://electropeak.com/learn/

*/

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define PIN      D6
#define NUMPIXELS 7


Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define DELAYVAL 500 // Time (in milliseconds) to pause between pixels

void setup() {
pixels.begin();
}

void loop() {
pixels.clear();
pixels.setBrightness(2);
pixels.setPixelColor(0, pixels.Color(255, 255, 255));
pixels.setPixelColor(1, pixels.Color(255, 0, 0));
pixels.setPixelColor(2, pixels.Color(0, 255, 0));
pixels.setPixelColor(3, pixels.Color(0, 0, 255));
pixels.setPixelColor(4, pixels.Color(255, 0, 255));
pixels.setPixelColor(5, pixels.Color(255, 255, 0));
pixels.setPixelColor(6, pixels.Color(0, 255, 255));
pixels.show();
}
