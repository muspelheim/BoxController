#include <Adafruit_NeoPixel.h>
#include <Servo.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN D3 // D3 is a pin for LED Strip

//Create new servo object to control box open/close
Servo servo;


int servoPosition = 0;
int servoPin = 2; // Arduino GIO2 equal D4 ESP Pin
int isAlreadyInited = 0;
int numberOfLedPixels = 24;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(numberOfLedPixels4, PIN, NEO_GRB + NEO_KHZ800);

//Setup runned once
void setup() 
{ 
  strip.setBrightness(100);
  strip.begin();
  strip.show();

  rainbowCycle(5); //first cycle before servo (change color every 5ms)
  servo.attach(servoPin);  // attaches the servo on GIO2 to the servo object 
} 

//Loop runned without stop
void loop() 
{ 
  if(isAlreadyInited < 1) {
    for(int i=0;i<numberOfLedPixels;i++)
    {
      // 220-20-60 is a red color
      strip.setPixelColor(i, strip.Color(220, 20, 60));
    } 
    strip.show();

    for(servoPosition = 90; servoPosition <= 180; servoPosition += 1)
    {
      servo.write(102); //servo speed (slow)
      delay(35); // send SIGNAL 35 ms
    } 
  
    servo.detach();

    isAlreadyInited = 1;

    // 65% strip brightness after opening
    strip.setBrightness(65);
  }

  rainbowCycle(20);
} 

//--------------------------------- LED ---------------------------------------
void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
//--------------------------------- END LED -----------------------------------
