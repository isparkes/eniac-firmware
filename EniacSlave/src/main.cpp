//**********************************************************************************
//* Main code for an Arduino based Nixie clock. Features:                          *
//*  - Real Time Clock interface for DS3231                                        *
//*  - Digit fading with configurable fade length                                  *
//*  - Digit scrollback with configurable scroll speed                             *
//*  - Configuration stored in EEPROM                                              *
//*  - Low hardware component count (as much as possible done in code)             *
//*  - Single button operation with software debounce                              *
//*  - Single K155ID1 for digit display (other versions use 2 or even 6!)          *
//*  - Automatic dimming, using a Light Dependent Resistor                         *
//*  - RGB back light management using individually addressable WS2812B            *
//*  - PIR sensor to turn off display when no one is around                        *
//*                                                                                *
//*  nixie@protonmail.ch                                                           *
//*                                                                                *
//**********************************************************************************
//**********************************************************************************
// Standard Libraries
#include <avr/io.h>
#include <Wire.h>
#include <avr/wdt.h>

#include <NeoPixelBus.h>        // https://github.com/Makuna/NeoPixelBus (Makuna 2.6.2)

// Other parts of the code, broken out for clarity
#include "DisplayDefs.h"

#define WS2812                  // WS2812, APA106
#define NORMAL_LEDS             // NORMAL_LEDS, FLIP_LEDS

// Software version shown in config menu
#define SOFTWARE_VERSION      001

#define I2C_SLAVE_ADDR                0x69

// Display handling
#define DIGIT_DISPLAY_COUNT   1000 // The number of times to traverse inner fade loop per digit
#define DIGIT_DISPLAY_ON      0    // Switch on the digit at the beginning by default
#define DIGIT_DISPLAY_OFF     999  // Switch off the digit at the end by default
#define DIGIT_DISPLAY_NEVER   -1   // When we don't want to switch on or off (i.e. blanking)
#define DISPLAY_COUNT_MAX     2000 // Maximum value we can set to
#define DISPLAY_COUNT_MIN     500  // Minimum value we can set to

// Dimming value
const int DIM_VALUE = DIGIT_DISPLAY_COUNT / 5;

#define BLINK_COUNT_MAX                25   // The number of impressions between blink state toggle

// How quickly the scroll works
#define SCROLL_STEPS_DEFAULT 4
#define SCROLL_STEPS_MIN     1
#define SCROLL_STEPS_MAX     40

// The number of dispay impessions we need to fade by default
// 100 is about 1 second
#define FADE_STEPS_DEFAULT 50
#define FADE_STEPS_MAX     200
#define FADE_STEPS_MIN     20

#define COLOUR_CNL_MAX                  15
#define COLOUR_RED_CNL_DEFAULT          15
#define COLOUR_GRN_CNL_DEFAULT          0
#define COLOUR_BLU_CNL_DEFAULT          0
#define COLOUR_CNL_MIN                  0

// Clock modes - normal running is MODE_TIME, other modes accessed by a middle length ( 1S < press < 2S ) button press
#define MODE_TIME                       0
#define MODE_MIN                        MODE_TIME

#define BLANK_MODE_MIN                  0
#define BLANK_MODE_TUBES                0  // Use blanking for tubes only 
#define BLANK_MODE_LEDS                 1  // Use blanking for LEDs only
#define BLANK_MODE_BOTH                 2  // Use blanking for tubes and LEDs
#define BLANK_MODE_MAX                  2
#define BLANK_MODE_DEFAULT              2

#define BACKLIGHT_MIN                   0
#define BACKLIGHT_FIXED                 0   // Just define a colour and stick to it
#define BACKLIGHT_PULSE                 1   // pulse the defined colour
#define BACKLIGHT_CYCLE                 2   // cycle through random colours
#define BACKLIGHT_FIXED_DIM             3   // A defined colour, but dims with bulb dimming
#define BACKLIGHT_PULSE_DIM             4   // pulse the defined colour, dims with bulb dimming
#define BACKLIGHT_CYCLE_DIM             5   // cycle through random colours, dims with bulb dimming
#define BACKLIGHT_COLOUR_TIME           6   // use "ColourTime" - different colours for each digit value
#define BACKLIGHT_COLOUR_TIME_DIM       7   // use "ColourTime" - dims with bulb dimming
#define BACKLIGHT_MAX                   7
#define BACKLIGHT_DEFAULT               4

#define CYCLE_SPEED_MIN                 4
#define CYCLE_SPEED_MAX                 64
#define CYCLE_SPEED_DEFAULT             10

//**********************************************************************************
//**********************************************************************************
//*                               Variables                                        *
//**********************************************************************************
//**********************************************************************************

// ***** Pin Defintions ****** Pin Defintions ****** Pin Defintions ******

// K155ID1
// These are now managed directly on PORT B, we don't use digitalWrite() for these
#define ledPin_0_a  13    // package pin 17 // PB5
#define ledPin_0_b  10    // package pin 14 // PB2
#define ledPin_0_c  8     // package pin 12 // PB0
#define ledPin_0_d  12    // package pin 16 // PB4

// anode pins
#define ledPin_a_1  7     // high - Hours tens  // package pin 11 // PD7
#define ledPin_a_2  4     //      - Hours units // package pin 2  // PD4
#define ledPin_a_3  3     //      - Mins  tens  // package pin 1  // PD3
#define ledPin_a_4  2     //      - Mins  units // package pin 32 // PD2

// PWM capable output for backlight - Neo Pixel chain
#define LED_DOUT    6     // package pin 12 // PD6

enum slaveDisplayModes {
  hundredthsMode,
  dateMode,
  secondsMode
};
slaveDisplayModes slaveDisplayMode = hundredthsMode;   

// ********************** HV generator variables *********************
// Used for special mappings of the K155ID1 -> digit (wiring aid)
// allows the board wiring to be much simpler
byte decodeDigit[10] = {2, 3, 7, 6, 4, 5, 1, 0, 9, 8};

// Driver pins for the anodes
byte anodePins[DIGIT_COUNT] = {ledPin_a_1, ledPin_a_2, ledPin_a_3, ledPin_a_4};

// ************************ Display management ************************
byte NumberArray[DIGIT_COUNT]     = {0, 0, 0, 0};
byte currNumberArray[DIGIT_COUNT] = {0, 0, 0, 0};
byte displayType[DIGIT_COUNT]     = {FADE, FADE, FADE, FADE};
byte fadeState[DIGIT_COUNT]       = {0, 0, 0, 0};
byte dateToShow;
byte monthToShow;
byte secondToShow;
byte hundredths;

// how many fade steps to increment (out of DIGIT_DISPLAY_COUNT) each impression
// 100 is about 1 second
int fadeSteps = FADE_STEPS_DEFAULT;
int digitOffCount = DIGIT_DISPLAY_OFF;
int scrollSteps = SCROLL_STEPS_DEFAULT;
boolean scrollback = true;
boolean fade = true;

int dispCount = DIGIT_DISPLAY_COUNT;
float fadeStep = DIGIT_DISPLAY_COUNT / fadeSteps;

byte blankMode = 0;

// For software blinking
int blinkCounter = 0;
boolean blinkState = true;

// leading digit blanking
boolean blankLeading = false;

boolean blankTubes = false;
boolean blankLEDs = false;

// State variables for detecting changes
unsigned long nowMillis = 0;
unsigned long lastCheckMillis = 0;
boolean triggeredThisSec = false;

boolean blanked = false;

// **************************** LED management ***************************
boolean upOrDown;

// Blinking colons led in settings modes
int ledBlinkCtr = 0;
int ledBlinkNumber = 0;

byte backlightMode = BACKLIGHT_DEFAULT;

// Back light intensities
byte redCnl = COLOUR_RED_CNL_DEFAULT;
byte grnCnl = COLOUR_GRN_CNL_DEFAULT;
byte bluCnl = COLOUR_BLU_CNL_DEFAULT;
byte cycleCount = 0;
byte cycleSpeed = CYCLE_SPEED_DEFAULT;

int colors[3];

// individual channel colours for the LEDs
byte ledR[DIGIT_COUNT];
byte ledG[DIGIT_COUNT];
byte ledB[DIGIT_COUNT];

// set up the NeoPixel library
#ifdef APA106
  NeoPixelBus<NeoRgbFeature, NeoSk6812Method> leds(DIGIT_COUNT, LED_DOUT);
#else
  NeoPixelBus<NeoGrbFeature, NeoWs2812Method> leds(DIGIT_COUNT, LED_DOUT);
#endif

// Strategy 3
int changeSteps = 0;
byte currentColour = 0;

// ************************************************************
// Set the tubes and LEDs blanking variables based on blanking mode and 
// blank mode settings
// ************************************************************
void setTubesAndLEDSBlankMode() {
  if (blanked) {
    switch(blankMode) {
      case BLANK_MODE_TUBES:
      {
        blankTubes = true;
        blankLEDs = false;
        break;
      }
      case BLANK_MODE_LEDS:
      {
        blankTubes = false;
        blankLEDs = true;
        break;
      }
      case BLANK_MODE_BOTH:
      {
        blankTubes = true;
        blankLEDs = true;
        break;
      }
    }
  } else {
    blankTubes = false;
    blankLEDs = false;
  }
}

// ************************************************************
// Called once per second
// ************************************************************
void performOncePerSecondProcessing() {
  // Change the direction of the pulse
  upOrDown = !upOrDown;

  setTubesAndLEDSBlankMode();

  // feed the watchdog
  wdt_reset();
}

// ************************************************************
// Put the led buffers out
// ************************************************************
void outputLEDBuffer() {
  for (int i = 0 ; i < DIGIT_COUNT ; i++) {
#ifdef NORMAL_LEDS
    leds.SetPixelColor(i, RgbColor(ledR[i], ledG[i], ledB[i]));
#else
    leds.SetPixelColor(i, RgbColor(ledR[DIGIT_COUNT - 1 - i], ledG[DIGIT_COUNT - 1 - i], ledB[DIGIT_COUNT - 1 - i]));
#endif
  }
  leds.Show();
}

// ************************************************************
// Set back light LEDs to the same colour
// ************************************************************
void setAllLEDs(byte red, byte green, byte blue) {
  for (int i = 0 ; i < DIGIT_COUNT ; i++) {
    ledR[i] = red;
    ledG[i] = green;
    ledB[i] = blue;
  }
  outputLEDBuffer();
}

// ************************************************************
// output a PWM LED channel, adjusting for dimming and PWM
// brightness:
// rawValue: The raw brightness value between 0 - 255
// ledPWMVal: The pwm factor between 0 - 1
// dimFactor: The dimming value between 0 - 1
// ************************************************************
byte getLEDAdjusted(float rawValue, float ledPWMVal, float dimFactor) {
  byte dimmedPWMVal = (byte)(rawValue * ledPWMVal * dimFactor);
  return dim_curve[dimmedPWMVal];
}

// ************************************************************
// Colour cycling 3: one colour dominates
// ************************************************************
void cycleColours3(int colors[3]) {
  cycleCount++;
  if (cycleCount > cycleSpeed) {
    cycleCount = 0;

    if (changeSteps == 0) {
      changeSteps = random(256);
      currentColour = random(3);
    }

    changeSteps--;

    switch (currentColour) {
      case 0:
        if (colors[0] < 255) {
          colors[0]++;
          if (colors[1] > 0) {
            colors[1]--;
          }
          if (colors[2] > 0) {
            colors[2]--;
          }
        } else {
          changeSteps = 0;
        }
        break;
      case 1:
        if (colors[1] < 255) {
          colors[1]++;
          if (colors[0] > 0) {
            colors[0]--;
          }
          if (colors[2] > 0) {
            colors[2]--;
          }
        } else {
          changeSteps = 0;
        }
        break;
      case 2:
        if (colors[2] < 255) {
          colors[2]++;
          if (colors[0] > 0) {
            colors[0]--;
          }
          if (colors[1] > 0) {
            colors[1]--;
          }
        } else {
          changeSteps = 0;
        }
        break;
    }
  }
}

// ************************************************************
// Set the seconds tick led(s) and the back lights
// ************************************************************
void setLeds()
{
  int secsTriangle;
  if (upOrDown) {
    secsTriangle = (nowMillis - lastCheckMillis);
  } else {
    secsTriangle = 1000 - (nowMillis - lastCheckMillis);
  }

  // calculate the PWM factor, goes between minDim% and 100%
  float dimFactor = (float) digitOffCount / (float) DIGIT_DISPLAY_OFF;
  float pwmFactor = (float) secsTriangle / (float) 1000.0;

  // ------------ Back light LEDs in normal mode ----------
  if (blankLEDs) {
          setAllLEDs( getLEDAdjusted(0, 1, 1), 
                      getLEDAdjusted(0, 1, 1), 
                      getLEDAdjusted(0, 1, 1));
  } else {
    switch (backlightMode) {
      case BACKLIGHT_FIXED:
        setAllLEDs( getLEDAdjusted(rgb_backlight_curve[redCnl], 1, 1), 
                    getLEDAdjusted(rgb_backlight_curve[grnCnl], 1, 1), 
                    getLEDAdjusted(rgb_backlight_curve[bluCnl], 1, 1));
        break;
      case BACKLIGHT_PULSE:
        setAllLEDs( getLEDAdjusted(rgb_backlight_curve[redCnl], pwmFactor, 1),
                    getLEDAdjusted(rgb_backlight_curve[grnCnl], pwmFactor, 1),
                    getLEDAdjusted(rgb_backlight_curve[bluCnl], pwmFactor, 1));
        break;
      case BACKLIGHT_CYCLE:
        cycleColours3(colors);
        setAllLEDs( getLEDAdjusted(colors[0], 1, 1),
                    getLEDAdjusted(colors[1], 1, 1),
                    getLEDAdjusted(colors[2], 1, 1));
        break;
      case BACKLIGHT_FIXED_DIM:
        setAllLEDs( getLEDAdjusted(rgb_backlight_curve[redCnl], 1, dimFactor), 
                    getLEDAdjusted(rgb_backlight_curve[grnCnl], 1, dimFactor), 
                    getLEDAdjusted(rgb_backlight_curve[bluCnl], 1, dimFactor));
        break;
      case BACKLIGHT_PULSE_DIM:
        setAllLEDs( getLEDAdjusted(rgb_backlight_curve[redCnl], pwmFactor, dimFactor),
                    getLEDAdjusted(rgb_backlight_curve[grnCnl], pwmFactor, dimFactor),
                    getLEDAdjusted(rgb_backlight_curve[bluCnl], pwmFactor, dimFactor));
        break;
      case BACKLIGHT_CYCLE_DIM:
        cycleColours3(colors);
        setAllLEDs( getLEDAdjusted(colors[0], 1, dimFactor),
                    getLEDAdjusted(colors[1], 1, dimFactor),
                    getLEDAdjusted(colors[2], 1, dimFactor));
        break;
      case BACKLIGHT_COLOUR_TIME:
          for (int i = 0 ; i < DIGIT_COUNT ; i++) {
            ledR[DIGIT_COUNT-1-i] = getLEDAdjusted(colourTimeR[NumberArray[i]],1,1);
            ledG[DIGIT_COUNT-1-i] = getLEDAdjusted(colourTimeG[NumberArray[i]],1,1);
            ledB[DIGIT_COUNT-1-i] = getLEDAdjusted(colourTimeB[NumberArray[i]],1,1);
          }
          outputLEDBuffer();
        break;
      case BACKLIGHT_COLOUR_TIME_DIM:
          for (int i = 0 ; i < DIGIT_COUNT ; i++) {
            ledR[DIGIT_COUNT-1-i] = getLEDAdjusted(colourTimeR[NumberArray[i]],1,dimFactor);
            ledG[DIGIT_COUNT-1-i] = getLEDAdjusted(colourTimeG[NumberArray[i]],1,dimFactor);
            ledB[DIGIT_COUNT-1-i] = getLEDAdjusted(colourTimeB[NumberArray[i]],1,dimFactor);
          }
          outputLEDBuffer();
        break;
    }
  }
}

//**********************************************************************************
//**********************************************************************************
//*                             Utility functions                                  *
//**********************************************************************************
//**********************************************************************************

// ************************************************************
// Break the date into displayable digits, respecting date format
// ************************************************************
void loadNumberArrayHundredths() {
  NumberArray[3] = hundredths % 10;
  NumberArray[2] = hundredths / 10;
  NumberArray[1] = 0;
  NumberArray[0] = 0;
}

// ************************************************************
// Show the same value on all digits
// ************************************************************
void loadNumberArrayDate() {
  NumberArray[3] = dateToShow%10;
  NumberArray[2] = dateToShow/10;
  NumberArray[1] = 0;
  NumberArray[0] = 0;
}

// ************************************************************
// Show the same value on all digits
// ************************************************************
void loadNumberArraySeconds() {
  NumberArray[3] = secondToShow%10;
  NumberArray[2] = secondToShow/10;
  NumberArray[1] = 0;
  NumberArray[0] = 0;
}

// ************************************************************
// Display preset
// ************************************************************
void allFade() {
  if (displayType[0] != FADE) displayType[0] = FADE;
  if (displayType[1] != FADE) displayType[1] = FADE;
  if (displayType[2] != FADE) displayType[2] = FADE;
  if (displayType[3] != FADE) displayType[3] = FADE;
}

// ************************************************************
// Display preset
// ************************************************************
void allNormal() {
  if (displayType[0] != NORMAL) displayType[0] = NORMAL;
  if (displayType[1] != NORMAL) displayType[1] = NORMAL;
  if (displayType[2] != NORMAL) displayType[2] = NORMAL;
  if (displayType[3] != NORMAL) displayType[3] = NORMAL;
}

// ************************************************************
// Display preset
// ************************************************************
void allFadeOrNormal(boolean blanking) {
  if (fade) {
    allFade();
  } else {
    allNormal();
  }
}

// ************************************************************
// Display preset
// ************************************************************
void allBlanked() {
  if (displayType[0] != BLANKED) displayType[0] = BLANKED;
  if (displayType[1] != BLANKED) displayType[1] = BLANKED;
  if (displayType[2] != BLANKED) displayType[2] = BLANKED;
  if (displayType[3] != BLANKED) displayType[3] = BLANKED;
}

// ************************************************************
// Decode the value to send to the 74141 and send it
// We do this via the decoder to allow easy adaptation to
// other pin layouts.
// ************************************************************
void SetSN74141Chip(int num1)
{
  // Map the logical numbers to the hardware pins we send to the SN74141 IC
  int decodedDigit = decodeDigit[num1];

  // Mask all digit bits to 0
  byte portb = PORTB;
  portb = portb & B11001010;

  // Set the bits we need
  switch ( decodedDigit )
  {
    case 0:                             break; // a=0;b=0;c=0;d=0
    case 1:  portb = portb | B00100000; break; // a=1;b=0;c=0;d=0
    case 2:  portb = portb | B00000100; break; // a=0;b=1;c=0;d=0
    case 3:  portb = portb | B00100100; break; // a=1;b=1;c=0;d=0
    case 4:  portb = portb | B00000001; break; // a=0;b=0;c=1;d=0
    case 5:  portb = portb | B00100001; break; // a=1;b=0;c=1;d=0
    case 6:  portb = portb | B00000101; break; // a=0;b=1;c=1;d=0
    case 7:  portb = portb | B00100101; break; // a=1;b=1;c=1;d=0
    case 8:  portb = portb | B00010000; break; // a=0;b=0;c=0;d=1
    case 9:  portb = portb | B00110000; break; // a=1;b=0;c=0;d=1
    default: portb = portb | B00110101; break; // a=1;b=1;c=1;d=1
  }
  PORTB = portb;
}

// ************************************************************
// Set a digit with the given value and turn the HVGen on
// Assumes that all digits have previously been turned off
// by a call to "digitOff"
// ************************************************************
void digitOn(int digit, int value) {
  switch (digit) {
    case 0: PORTD = PORTD | B10000000; break; // PD7 - equivalent to digitalWrite(ledPin_a_1,HIGH);
    case 1: PORTD = PORTD | B00010000; break; // PD4 - equivalent to digitalWrite(ledPin_a_2,HIGH);
    case 2: PORTD = PORTD | B00001000; break; // PD3 - equivalent to digitalWrite(ledPin_a_3,HIGH);
    case 3: PORTD = PORTD | B00000100; break; // PD2 - equivalent to digitalWrite(ledPin_a_4,HIGH);
  }
  SetSN74141Chip(value);
}

// ************************************************************
// Finish displaying a digit
// ************************************************************
void digitOff() {
  // turn all digits off - equivalent to digitalWrite(ledPin_a_n,LOW); (n=1,2,3,4) but much faster
  PORTD = PORTD & B01100011;
}

// ************************************************************
// Do a single complete display, including any fading and
// dimming requested. Performs the display loop
// DIGIT_DISPLAY_COUNT times for each digit, with no delays.
// This is the heart of the display processing!
// ************************************************************
void outputDisplay()
{
  int digitOnTime;
  int digitOffTime;
  int digitSwitchTime;
  int tmpDispType;

  for ( int i = 2 ; i < DIGIT_COUNT ; i ++ )
  {
    if (blankTubes) {
      tmpDispType = BLANKED;
    } else {
      tmpDispType = displayType[i];
    }

    switch (tmpDispType) {
      case BLANKED:
        {
          digitOnTime = DIGIT_DISPLAY_NEVER;
          digitOffTime = DIGIT_DISPLAY_ON;
          break;
        }
      case DIMMED:
        {
          digitOnTime = DIGIT_DISPLAY_ON;
          digitOffTime = DIM_VALUE;
          break;
        }
      case BRIGHT:
        {
          digitOnTime = DIGIT_DISPLAY_ON;
          digitOffTime = DIGIT_DISPLAY_OFF;
          break;
        }
      case FADE:
      case NORMAL:
        {
          digitOnTime = DIGIT_DISPLAY_ON;
          digitOffTime = digitOffCount;
          break;
        }
      case BLINK:
        {
          if (blinkState) {
            digitOnTime = DIGIT_DISPLAY_ON;
            digitOffTime = digitOffCount;
          } else {
            digitOnTime = DIGIT_DISPLAY_NEVER;
            digitOffTime = DIGIT_DISPLAY_ON;
          }
          break;
        }
      case SCROLL:
        {
          digitOnTime = DIGIT_DISPLAY_ON;
          digitOffTime = digitOffCount;
          break;
        }
    }

    // Do scrollback when we are going to 0
    if ((NumberArray[i] != currNumberArray[i]) &&
        (NumberArray[i] == 0) &&
        scrollback) {
      tmpDispType = SCROLL;
    }

    // manage fading, each impression we show 1 fade step less of the old
    // digit and 1 fade step more of the new
    // manage fading, each impression we show 1 fade step less of the old
    // digit and 1 fade step more of the new
    if (tmpDispType == SCROLL) {
      digitSwitchTime = DIGIT_DISPLAY_OFF;
      if (NumberArray[i] != currNumberArray[i]) {
        if (fadeState[i] == 0) {
          // Start the fade
          fadeState[i] = scrollSteps;
        }

        if (fadeState[i] == 1) {
          // finish the fade
          fadeState[i] = 0;
          currNumberArray[i] = currNumberArray[i] - 1;
        } else if (fadeState[i] > 1) {
          // Continue the scroll countdown
          fadeState[i] = fadeState[i] - 1;
        }
      }
    } else if (tmpDispType == FADE) {
      if (NumberArray[i] != currNumberArray[i]) {
        if (fadeState[i] == 0) {
          // Start the fade
          fadeState[i] = fadeSteps;
          digitSwitchTime = (int) fadeState[i] * fadeStep;
        }
      }

      if (fadeState[i] == 1) {
        // finish the fade
        fadeState[i] = 0;
        currNumberArray[i] = NumberArray[i];
        digitSwitchTime = DIGIT_DISPLAY_COUNT;
      } else if (fadeState[i] > 1) {
        // Continue the fade
        fadeState[i] = fadeState[i] - 1;
        digitSwitchTime = (int) fadeState[i] * fadeStep;
      }
    } else {
      digitSwitchTime = DIGIT_DISPLAY_COUNT;
      currNumberArray[i] = NumberArray[i];
    }

    for (int timer = 0 ; timer < dispCount ; timer++) {
      if (timer == digitOnTime) {
        digitOn(i, currNumberArray[i]);
      }

      if  (timer == digitSwitchTime) {
        SetSN74141Chip(NumberArray[i]);
      }

      if (timer == digitOffTime) {
        digitOff();
      }
    }
  }

  // Deal with blink, calculate if we are on or off
  blinkCounter++;
  if (blinkCounter == BLINK_COUNT_MAX) {
    blinkCounter = 0;
    blinkState = !blinkState;
  }
}

//**********************************************************************************
//**********************************************************************************
//*                                 I2C interface                                  *
//**********************************************************************************
//**********************************************************************************

#define I2C_SET_SLAVE_DATA     0x1A
#define I2C_SET_SLAVE_DATE     0x1B
#define I2C_SET_SLAVE_SECS     0x1C
/**
 * receive information from the master
 */
void receiveEvent(int bytes) {
  // the operation tells us what we are getting
  int operation = Wire.read();
    secondToShow = Wire.read();
    dateToShow = Wire.read();
    monthToShow = Wire.read();

  switch (operation) {
    case I2C_SET_SLAVE_DATA: {
      slaveDisplayMode = hundredthsMode;
      break;      
    }
    case I2C_SET_SLAVE_DATE: {
      slaveDisplayMode = dateMode;
      break;      
    }
    case I2C_SET_SLAVE_SECS: {
      slaveDisplayMode = secondsMode;
      break;      
    }
  }
}

/**
   send information to the master
*/
void requestEvent() {
}

byte encodeBooleanForI2C(boolean valueToProcess) {
  if (valueToProcess) {
    byte byteToSend = 1;
    return byteToSend;
  } else {
    byte byteToSend = 0;
    return byteToSend;
  }
}

//**********************************************************************************
//**********************************************************************************
//*                                    Setup                                       *
//**********************************************************************************
//**********************************************************************************
void setup()
{
  pinMode(ledPin_0_a, OUTPUT);
  pinMode(ledPin_0_b, OUTPUT);
  pinMode(ledPin_0_c, OUTPUT);
  pinMode(ledPin_0_d, OUTPUT);

  pinMode(ledPin_a_1, OUTPUT);
  pinMode(ledPin_a_2, OUTPUT);
  pinMode(ledPin_a_3, OUTPUT);
  pinMode(ledPin_a_4, OUTPUT);

  setAllLEDs(0,0,0);

  // **********************************************************************

  // Set up the LED output
  leds.Begin();

  // reset the LEDs
  setLeds();

  // initialise the internal time (in case we don't find the time provider)
  nowMillis = millis();

  Wire.begin(I2C_SLAVE_ADDR);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  // enable watchdog
  wdt_enable(WDTO_8S);
}

//**********************************************************************************
//**********************************************************************************
//*                              Main loop                                         *
//**********************************************************************************
//**********************************************************************************
void loop()
{
  nowMillis = millis();

  // -------------------------------------------------------------------------------

  if (abs(nowMillis - lastCheckMillis) >= 1000) {
    if (!triggeredThisSec) {
      performOncePerSecondProcessing();
    }

    // Make sure we don't call multiple times
    triggeredThisSec = true;

    lastCheckMillis = nowMillis;
  }
  
  // -------------------------------------------------------------------------------

  hundredths++;
  if (hundredths > 99) hundredths = 0;

  // -------------------------------------------------------------------------------

  // get the LDR ambient light reading
  fadeStep = digitOffCount / fadeSteps;

  switch (slaveDisplayMode) {
    case hundredthsMode: {
      loadNumberArrayHundredths();
      break;
    }
    case dateMode: {
      loadNumberArrayDate();
      break;      
    }
    case secondsMode: {
      loadNumberArraySeconds();
      break;      
    }
  }
  outputDisplay();

  // Prepare the tick and backlight LEDs
  setLeds();
}

