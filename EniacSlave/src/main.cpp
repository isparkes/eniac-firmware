//**********************************************************************************
//* ENIAC Slave                                                                    *
//**********************************************************************************
// Standard Libraries
#include <avr/io.h>
#include <Wire.h>
//#include <avr/wdt.h>
#include <TimeLib.h>            // https://playground.arduino.cc/Code/Time/ (Margolis 1.5.0)

#include <NeoPixelBus.h>        // https://github.com/Makuna/NeoPixelBus (Makuna 2.6.2)

// Other parts of the code, broken out for clarity
#include "DisplayDefs.h"

// Use the NeoPixels to tell us about the I2C status
#define DIAG_BACKLIGHTS_OFF

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
#define BACKLIGHT_DEFAULT               3

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
  secondsMode,
  offMode
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
byte dimming;
byte secondToShow;
int hundredths;

// how many fade steps to increment (out of DIGIT_DISPLAY_COUNT) each impression
// 100 is about 1 second
int fadeSteps = FADE_STEPS_DEFAULT;
int digitOffCount = DIGIT_DISPLAY_OFF;
int scrollSteps = SCROLL_STEPS_DEFAULT;
boolean scrollback = false;
boolean fade = false;

int dispCount = DIGIT_DISPLAY_COUNT;
float fadeStep = DIGIT_DISPLAY_COUNT / fadeSteps;

// For software blinking
int blinkCounter = 0;
boolean blinkState = true;

// State variables for detecting changes
unsigned long nowMillis = 0;
unsigned long lastCheckMillis = 0;
unsigned long hundredthsMillis = 0;
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
// Set back light LEDs to the same colour
// ************************************************************
void setLED(byte i, byte red, byte green, byte blue) {
  ledR[i] = red;
  ledG[i] = green;
  ledB[i] = blue;
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
  displayType[0] = FADE;
  displayType[1] = FADE;
  displayType[2] = FADE;
  displayType[3] = FADE;
}

// ************************************************************
// Display preset
// ************************************************************
void allNormal() {
  displayType[0] = NORMAL;
  displayType[1] = NORMAL;
  displayType[2] = NORMAL;
  displayType[3] = NORMAL;
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
  displayType[0] = BLANKED;
  displayType[1] = BLANKED;
  displayType[2] = BLANKED;
  displayType[3] = BLANKED;
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
    if (blanked) {
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

#define SLAVE_MODE_100THS               0
#define SLAVE_MODE_DATE                 1
#define SLAVE_MODE_SECS                 2
#define SLAVE_MODE_OFF                  3

/**
 * receive information from the master
 */
void receiveEvent(int receivedBytes) {

  #ifdef DIAG_BACKLIGHTS
  setLED(3,0,255,0);
  #endif

  if (receivedBytes == 5) {
    #ifdef DIAG_BACKLIGHTS
    setLED(2,0,255,0);
    #endif

    // if we got an update, just say that it is a new second
    // for the 100ths
    hundredthsMillis = nowMillis;

    // resync the per second update, so that the "seconds" align 
    // between main and slave
    lastCheckMillis = nowMillis;

    byte mode = Wire.read();
    byte dimming  = Wire.read();
    secondToShow = Wire.read();
    dateToShow = Wire.read();
    monthToShow = Wire.read();

    #ifdef DIAG_BACKLIGHTS
    setLED(1,0,255,0);
    #endif

    // detect the blanking status
    blanked = (dimming == 0);

    // set the dimming
    digitOffCount = dimming * 10;
    if (digitOffCount > DIGIT_DISPLAY_OFF) digitOffCount = DIGIT_DISPLAY_OFF;

    // we got a transmission
    switch (mode) {
      case SLAVE_MODE_100THS: {
        slaveDisplayMode = hundredthsMode;
        break;      
      }
      case SLAVE_MODE_SECS: {
        slaveDisplayMode = secondsMode;
        break;      
      }
      case SLAVE_MODE_DATE: {
        slaveDisplayMode = dateMode;
        break;      
      }
      case SLAVE_MODE_OFF: {
        slaveDisplayMode = offMode;
        break;      
      }
    }
    #ifdef DIAG_BACKLIGHTS
    setLED(0,0,255,0);
    #endif
  } else {
    #ifdef DIAG_BACKLIGHTS
    setLED(2,255,0,0);
    #endif
  }
}

// ************************************************************
// Called once per second
// ************************************************************
void performOncePerSecondProcessing() {
  // Change the direction of the pulse
  upOrDown = !upOrDown;

  secondToShow++;
  if (secondToShow > 59) {
    secondToShow = 0;
  }

  #ifdef DIAG_BACKLIGHTS
  setAllLEDs(0,0,0);
  #endif

  // setTubesAndLEDSBlankMode();

  // feed the watchdog
//  wdt_reset();
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

  // **********************************************************************

  delay(100);

  // Set up the LED output
  leds.Begin();

  delay(100);

  // reset the LEDs
  // setLeds();

  // initialise the internal time (in case we don't find the time provider)
  nowMillis = millis();

  setAllLEDs(0,0,0);

  Wire.begin(I2C_SLAVE_ADDR);
  Wire.onReceive(receiveEvent);

  // enable watchdog
//  wdt_enable(WDTO_8S);
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
    performOncePerSecondProcessing();
    lastCheckMillis = nowMillis;
  }
  
  // -------------------------------------------------------------------------------

  unsigned long hundredthsDelta = (nowMillis - hundredthsMillis)/10;
  hundredthsDelta = hundredthsDelta % 100;
  hundredths = (byte) hundredthsDelta;

  // -------------------------------------------------------------------------------

  // get the LDR ambient light reading
  fadeStep = digitOffCount / fadeSteps;

  switch (slaveDisplayMode) {
    case hundredthsMode: {
      allNormal();
      loadNumberArrayHundredths();
      break;
    }
    case dateMode: {
      allNormal();
      loadNumberArrayDate();
      break;      
    }
    case secondsMode: {
      allNormal();
      loadNumberArraySeconds();
      break;      
    }
    case offMode: {
      allBlanked();
      break;      
    }
  }
  outputDisplay();
}

