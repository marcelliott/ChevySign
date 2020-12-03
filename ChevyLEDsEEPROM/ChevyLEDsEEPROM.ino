#include <EEPROM.h>

#include <Adafruit_NeoPixel.h>

#define LED_OUTPUT_PIN 6
#define MUSIC_PLAYING_INPUT_PIN 4
#define   MUSIC_ACTIVE 0
#define NUM_LEDS 11
#define NUM_HL_LEDS 4
#define NUM_GRILL_LEDS (NUM_LEDS - NUM_HL_LEDS)
#define ON_BOARD_LED_PIN 13
#define POT1_PIN A0
#define POT2_PIN A1
#define POT3_PIN A2

#define BRIGHTNESS_MIN 0
#define BRIGHTNESS_MAX 255
#define BRIGHTNESS_DEFAULT 50

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_OUTPUT_PIN, NEO_GRB + NEO_KHZ800);

// Globals
uint8_t brightness = BRIGHTNESS_DEFAULT;
int pot1_val;
int pot2_val;
int pot3_val;

uint8_t mode = 0;
#define NUM_MODES 10

// EEPROM
#define EEPROM_MODE_OFFSET 0
#define EEPROM_BRIGHTNESS_OFFSET 1

void setup() {
  Serial.begin(9600);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  // Read last mode store in EEPROM
  //mode = EEPROM.read(EEPROM_MODE_OFFSET);
  brightness = EEPROM.read(EEPROM_BRIGHTNESS_OFFSET);
  Serial.print("Mode read from EEPROM: ");
  Serial.println(mode);
  
  delay(5000);  // give enough time for MUSIC Arduino to finish startup
  Serial.println("SETUP complete");
}

void loop() {
  static uint8_t lastMode = mode;
  static int musicPlayingState = 0;
  // read the pin from the Robertson Sound Card to indicate if track is playing
  musicPlayingState = digitalRead(MUSIC_PLAYING_INPUT_PIN);

  // read the potentiometers (pots)
  int new_pot_val;
  new_pot_val = analogRead(POT1_PIN);
  new_pot_val = map(new_pot_val, 0, 1023, 0, 255);
  if (abs(new_pot_val - pot1_val) > 4) pot1_val = new_pot_val;
  new_pot_val = analogRead(POT2_PIN);
  new_pot_val = map(new_pot_val, 0, 1023, 0, 255);
  if (abs(new_pot_val - pot2_val) > 4) pot2_val = new_pot_val;
  new_pot_val = analogRead(POT3_PIN);
  new_pot_val = map(new_pot_val, 0, 1023, 0, 255);
  if (abs(new_pot_val - pot3_val) > 4) pot3_val = new_pot_val;

   
  //Serial.print("Pots");
  //Serial.print(pot1_val);
  //Serial.print(", ");
  //Serial.print(pot2_val);
  //Serial.print(", ");
  //Serial.print(pot3_val);
  //Serial.println("");

  if (musicPlayingState == MUSIC_ACTIVE) {
      // Max brightness when music playing
      pot1_val = 255;
  }
  
  if (pot1_val != brightness) {
    brightness = pot1_val;
    strip.setBrightness(brightness);
    EEPROM.write(EEPROM_BRIGHTNESS_OFFSET, brightness);
  }
   
  //Serial.print("Brightness: ");
  //Serial.println(brightness);
  //Serial.print("Mode: ");
  //Serial.println(mode);
  
  int i;
  for(i=0; i < NUM_HL_LEDS; i++) {
     if (musicPlayingState == MUSIC_ACTIVE) {
        strip.setPixelColor(i, Wheel(random(0,255)));
     }
     else {
        strip.setPixelColor(i, Wheel((pot2_val) & 255));
     }
  }   
  for(; i < NUM_LEDS; i++) {
     if (musicPlayingState == MUSIC_ACTIVE) {
        strip.setPixelColor(i, Wheel(random(0,255)));
     }
     else {
        strip.setPixelColor(i, Wheel((pot3_val) & 255));
     }
  }       

  strip.show(); 

  delay(100);
  //Serial.println("colorWipe Black (off)");
  //colorWipe(strip.Color(0, 0, 0), 50); // Black
  //delay(200);
 
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

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

// Slightly different, this makes the rainbow equally distributed throughout
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

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();
     
      delay(wait);
     
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
        }
        strip.show();
       
        delay(wait);
       
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, 0);        //turn every third pixel off
        }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
