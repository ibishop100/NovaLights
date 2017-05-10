#include "Button.h"                                           // https://github.com/JChristensen/Button
#include "EEPROM.h"
#include "FastLED.h"

#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif


// Fixed definitions cannot change on the fly.
#define LED_DT 6                                             // Data pin to connect to the strip.
#define COLOR_ORDER GRB                                       // It's GRB for WS2812 and BGR for APA102.
#define LED_TYPE WS2812                                       // Using APA102, WS2812, WS2801. Don't forget to modify LEDS.addLeds to suit.
#define NUM_LEDS 288                                           // Number of LED's.


#define BUTTON_PIN 2                                         // Connect a buttonor something similar from Arduino pin 6 to ground.
#define PULLUP true                                           // To keep things simple, we use the Arduino's internal pullup resistor.
#define INVERT true                                           // Since the pullup resistor will keep the pin high unless the
                                                              // switch is closed, this is negative logic, i.e. a high state
                                                              // means the button is NOT pressed. (Assuming a normally open switch.)
#define DEBOUNCE_MS 50                                        // A debounce time of 50 milliseconds seems to work well.


// Button variables
Button myBtn(BUTTON_PIN, PULLUP, INVERT, DEBOUNCE_MS);        //Declare the button
boolean longpress;

// Definition for the array of routines to display.
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

//Mode and EEPROM variables
//uint8_t ledMode = 0;
//uint8_t maxMode = 6;
int eepaddress = 0;

// Global variables can be changed on the fly.
uint8_t max_bright = 128;                                      // Overall brightness definition. It can be changed on the fly.

struct CRGB leds[NUM_LEDS];                                   // Initialize our LED array.

uint8_t gCurrentPatternNumber = 0;                            // Index number of which pattern is current
uint8_t gHue = 0;                                             // rotating "base color" used by many of the patterns

typedef void (*SimplePatternList[])();                        // List of patterns to cycle through.  Each is defined as a separate function below.

void setup() {

  Serial.begin(57600);                                        // Initialize serial port for debugging.
  delay(1000);                                                // Soft startup to ease the flow of electrons.

  LEDS.addLeds<LED_TYPE, LED_DT, COLOR_ORDER>(leds, NUM_LEDS);  // Use this for WS2812

  FastLED.setBrightness(max_bright);
  set_max_power_in_volts_and_milliamps(5, 500);               // FastLED Power management set at 5V, 500mA.

  gCurrentPatternNumber = EEPROM.read(eepaddress);
  Serial.print("Starting ledMode: ");
  Serial.println(gCurrentPatternNumber);

} // setup()

SimplePatternList gPatterns = {rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };

void loop() {
  readbutton();

  EVERY_N_MILLISECONDS(50) {
    gPatterns[gCurrentPatternNumber]();                         // Call the current pattern function once, updating the 'leds' array
  }

  EVERY_N_MILLISECONDS(20) {                                    // slowly cycle the "base color" through the rainbow
    gHue++;
  }

  FastLED.show();

} // loop()

void readbutton() {                                        // Read the button and increase the mode
  myBtn.read();

  if(myBtn.wasReleased()) {
    Serial.print("loop");
    if (longpress==1) {
      EEPROM.write(eepaddress,gCurrentPatternNumber);
      Serial.print("Writing: ");
    } else {
      gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE(gPatterns);
    }
    longpress = 0;
    Serial.println(gCurrentPatternNumber);
  }

  if(myBtn.pressedFor(1000)) {
    longpress = 1;
  }

} // readbutton()

//--------------------[ Effects are below here ]------------------------------------------------------------------------------

void rainbow() {

  fill_rainbow(leds, NUM_LEDS, gHue, 7);                       // FastLED's built-in rainbow generator.

} // rainbow()



void rainbowWithGlitter() {

  rainbow();                                                    // Built-in FastLED rainbow, plus some random sparkly glitter.
  addGlitter(80);

} // rainbowWithGlitter()



void addGlitter(fract8 chanceOfGlitter) {

  if(random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }

} // addGlitter()

void confetti() {                                             // Random colored speckles that blink in and fade smoothly.

  fadeToBlackBy(leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV(gHue + random8(64), 200, 255);

} // confetti()

void sinelon() {                                              // A colored dot sweeping back and forth, with fading trails.

  fadeToBlackBy(leds, NUM_LEDS, 20);
  int pos = beatsin16(13,0,NUM_LEDS);
  leds[pos] += CHSV(gHue, 255, 192);

} // sinelon()



void bpm() {                                                // Colored stripes pulsing at a defined Beats-Per-Minute.

  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);

  for(int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }

} // bpm()



void juggle() {                                             // Eight colored dots, weaving in and out of sync with each other.

  fadeToBlackBy(leds, NUM_LEDS, 20);
  byte dothue = 0;

  for(int i = 0; i < 8; i++) {
    leds[beatsin16(i+7,0,NUM_LEDS)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }

} // juggle()
