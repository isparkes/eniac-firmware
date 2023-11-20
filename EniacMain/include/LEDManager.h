#pragma once

#include "Arduino.h"
#include "Globals.h"
#include "SpiffsStorage.h"
#include "DebugManager.h"
#include "OutputManager.h"

#include "QuoteManager.h"

#include <NeoPixelBus.h>            // https://github.com/Makuna/NeoPixelBus (Makuna 2.6.6)

#define NUM_BL_PIXELS DIGIT_COUNT*PIXELS_PER_TUBE

#ifdef FEATURE_EXT_LEDS
  #define NUM_UL_PIXELS DIGIT_COUNT
#else
  #define NUM_UL_PIXELS 0
#endif

// FEATURE_SEP_LED: Two additional Neopixels in separator towers
// DIGIT_COUNT Backlights and DIGIT_COUNT underlights + 2 separators in towers
// The array LED_ADDR spreads the neopixels out if needed
#ifdef FEATURE_SEP_LED
  #define NUM_PIXELS_TOTAL NUM_BL_PIXELS+NUM_UL_PIXELS+2
  #define TOWER_1 4
  #define TOWER_2 9
#else
  #define NUM_PIXELS_TOTAL NUM_BL_PIXELS+NUM_UL_PIXELS
#endif

// --------------------------- Strategy Backlights -------------------------------
#define BACKLIGHT_MIN                   0
#define BACKLIGHT_FIXED                 0  // Just define a colour and stick to it
#define BACKLIGHT_CYCLE                 1  // cycle through random colours, strategy 3
#define BACKLIGHT_COLOUR_TIME           2  // use "ColourTime" - different colours for each digit value
#define BACKLIGHT_DAY_OF_WEEK           3  // use "ColourTime" - different colours for each digit value
#define BACKLIGHT_MAX                   3
#define BACKLIGHT_DEFAULT               1
#ifdef FEATURE_TICKER
#define BACKLIGHT_UP                  254  // Used for ticker display
#define BACKLIGHT_DOWN                253  //            "
#define BACKLIGHT_UNCHANGED           252  //            "
#endif

// -------------------------------------------------------------------------------
#define CYCLE_SPEED_MIN                 1
#define CYCLE_SPEED_MAX                 10
#define CYCLE_SPEED_DEFAULT             5

// -------------------------------------------------------------------------------
#define COLOUR_CNL_MAX                  15
#define COLOUR_RED_CNL_DEFAULT          15
#define COLOUR_GRN_CNL_DEFAULT          0
#define COLOUR_BLU_CNL_DEFAULT          0
#define COLOUR_CNL_MIN                  0

// -------------------------------------------------------------------------------
#define STATUS_RED    0
#define STATUS_YELLOW 1
#define STATUS_GREEN  2
#define STATUS_BLUE   3

// -------------------------------------------------------------------------------
#define BACKLIGHT_DIM_FACTOR_MIN        10
#define BACKLIGHT_DIM_FACTOR_MAX        100
#define BACKLIGHT_DIM_FACTOR_DEFAULT    100

// -------------------------------------------------------------------------------
#define EXT_DIM_FACTOR_MIN              10
#define EXT_DIM_FACTOR_MAX              100
#define EXT_DIM_FACTOR_DEFAULT          100

// -------------------------------------------------------------------------------
#define HUE_OFFSET_MIN                  0
#define HUE_OFFSET_MAX                  360
#define HUE_OFFSET_DEFAULT              30

// -------------------------------------------------------------------------------
#define LED_MODE_MIN        0
#define LED_RAILROAD        0
#define LED_BLINK_SLOW      1
#define LED_BLINK_FAST      2
#define LED_BLINK_DBL       3
#define LED_ON              4
#define LED_OFF             5
#define LED_BLINK_DEFAULT   LED_RAILROAD
#define LED_RAILROAD_X      -1 // Not yet implemented
#define LED_MODE_MAX        5

// -------------------------------------------------------------------------------
#define STATUS_LED_MODE_MIN    0
#define STATUS_LED_MODE_NTP    0
#define STATUS_LED_MODE_ONLINE 1
#define STATUS_LED_MODE_AM_PM  2
#define STATUS_LED_MODE_PIR    3
#define STATUS_LED_MODE_OFF    4
#define STATUS_LED_MODE_ON     5
#define STATUS_LED_MODE_MAX    5

// -------------------------------------------------------------------------------
#define SEPARATOR_DIM_FACTOR_MIN        10
#define SEPARATOR_DIM_FACTOR_MAX        100
#define SEPARATOR_DIM_FACTOR_DEFAULT    100

// -------------------------------------------------------------------------------
#define BACKLIGHT_DIM_FACTOR_MIN        10
#define BACKLIGHT_DIM_FACTOR_MAX        100
#define BACKLIGHT_DIM_FACTOR_DEFAULT    100

// ************************** Pin Allocations *************************

class LEDManager_
{
  private:
    LEDManager_() = default; // Make constructor private

  public:
    static LEDManager_ &getInstance(); // Accessor for singleton instance

    LEDManager_(const LEDManager_ &) = delete; // no copying
    LEDManager_ &operator=(const LEDManager_ &) = delete;

  public:
    void setUp();

    // recalculate internal values based on the LDR reading
    void setLDRValue(unsigned int ldrValue);

    // recalculate internal values based on the LDR reading
    void setLDRRange(unsigned int ldrRange);

    #ifdef FEATURE_TICKER
    // Allow the ticker to control the colours
    void setTickerOverrideValue(quote_direction newValue);
    #endif

    void setSyncColourTime(boolean value);
    void setDiagnosticLED(byte stepNumber, byte state);

    // Set the LEDs to a test value
    void setTestValue(byte value);

    // Use colour inverting for the second LED of each tube
    void setInvertLEDs(boolean value);

    // Set the blanking status
    void setLEDBlanking(boolean newStatus);
    void setTowerBlanking(boolean newStatus);

    void updateOncePerLoop();
    void updateOncePerSecond();
  private:
    float _backlightDim = 1.0;
    float _underlightDim = 1.0;
    float _ldrDimFactor = 1.0;
    float _ldrRange = 100.0;
    float _pulseFactor = 1.0;
    byte  _ledMode = BACKLIGHT_DEFAULT;
    byte  _cycleCount = 0;
    byte  _cycleSpeed = CYCLE_SPEED_DEFAULT;
    bool  _syncColourTime = false;
    byte  _dow = 0;
    float _hueOffset = 0.0;
    float _towerHueOffset = 0.0;

    // Processing optimisation - reduce the number of multiplications per loop
    // We cache these values each time one of them changes
    float _overallBLDimFactor;
    float _overallULDimFactor;
    #ifdef FEATURE_TICKER
    quote_direction  _tickerOverride = unchanged;
    #endif

    bool _blanked = false;
    bool _towersBlanked = false;

    // Strategy 3
    int _changeSteps = 0;
    byte _currentColour = 0;
    
    int _colors[3];

    // Back lights
    byte _ledRb[NUM_PIXELS_TOTAL];
    byte _ledGb[NUM_PIXELS_TOTAL];
    byte _ledBb[NUM_PIXELS_TOTAL];

    // Under lights
    byte _ledRu[DIGIT_COUNT];
    byte _ledGu[DIGIT_COUNT];
    byte _ledBu[DIGIT_COUNT];

    // hue offsets
    double _hueOffsetPerPixel[NUM_PIXELS_TOTAL];

    void setBacklightLEDs(byte red, byte green, byte blue);
    void setBacklightLEDsUnadjusted(byte red, byte green, byte blue);
    void setUnderlightLEDs(byte red, byte green, byte blue);
    void setBacklightLED(byte index, byte red, byte green, byte blue);
    void setBacklightLEDUnadjusted(byte index, byte red, byte green, byte blue);
    void setUnderlightLED(byte index, byte red, byte green, byte blue);
    void setTowerLEDs(byte red, byte green, byte blue);
    void outputLEDBuffer();
    byte getLEDAdjustedBL(byte rawValue);
    byte getLEDAdjustedUL(byte rawValue);
    byte getLEDRawBL(byte rawValue);

    // Calculate the colours when we are cycling
    void cycleColours3(int colors[3]);

    // Apply "hue" and "rainbow" processing
    void adjustRGB(uint8_t red, uint8_t green, uint8_t blue, uint8_t& inv_red, uint8_t& inv_green, uint8_t& inv_blue, double hueOffset);

    // reset internal values based on slow-moving values (e.g. backlight dim factor)
    void recalculateVariables();

    // This processes the values and outputs the buffer fr fast moving animations
    void processLedStatusLoop();

    // This processes the values and outputs the buffer for slow moving animations
    void processLedStatusOncePerSecond();

    // recalculate internal values based on the pulsing factor
    void setPulseValue();
};

// ************************************************************
// LED brightness correction: The perceived brightness is not linear
// ************************************************************
const byte dim_curve[] = {
  0,   1,   1,   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,   3,   3,
  3,   3,   3,   3,   3,   3,   3,   4,   4,   4,   4,   4,   4,   4,   4,   4,
  4,   4,   4,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   6,   6,   6,
  6,   6,   6,   6,   6,   7,   7,   7,   7,   7,   7,   7,   8,   8,   8,   8,
  8,   8,   9,   9,   9,   9,   9,   9,   10,  10,  10,  10,  10,  11,  11,  11,
  11,  11,  12,  12,  12,  12,  12,  13,  13,  13,  13,  14,  14,  14,  14,  15,
  15,  15,  16,  16,  16,  16,  17,  17,  17,  18,  18,  18,  19,  19,  19,  20,
  20,  20,  21,  21,  22,  22,  22,  23,  23,  24,  24,  25,  25,  25,  26,  26,
  27,  27,  28,  28,  29,  29,  30,  30,  31,  32,  32,  33,  33,  34,  35,  35,
  36,  36,  37,  38,  38,  39,  40,  40,  41,  42,  43,  43,  44,  45,  46,  47,
  48,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,
  63,  64,  65,  66,  68,  69,  70,  71,  73,  74,  75,  76,  78,  79,  81,  82,
  83,  85,  86,  88,  90,  91,  93,  94,  96,  98,  99,  101, 103, 105, 107, 109,
  110, 112, 114, 116, 118, 121, 123, 125, 127, 129, 132, 134, 136, 139, 141, 144,
  146, 149, 151, 154, 157, 159, 162, 165, 168, 171, 174, 177, 180, 183, 186, 190,
  193, 196, 200, 203, 207, 211, 214, 218, 222, 226, 230, 234, 238, 242, 248, 255,
};

const byte rgb_backlight_curve[] = {0, 16, 32, 48, 64, 80, 99, 112, 128, 144, 160, 176, 192, 216, 240, 255};

// Used to define "colourTime" colours
//                             0    1    2    3    4    5    6    7    8    9
const byte colourTimeR[] = { 255, 255, 204,  51,   0,   0,   0, 153, 204, 255};
const byte colourTimeG[] = {   0, 153, 192, 192, 255, 192, 102,   0,   0,   0};
const byte colourTimeB[] = {   0,   0,   0,   0,  51, 192, 255, 255, 255, 153};

// Used to define "colourTime" colours - Mia's 
//                             0    1    2    3    4    5    6    7    8    9
//const byte colourTimeR[] = { 255,   0, 255,  20, 255, 153,   0, 255,   0, 255};
//const byte colourTimeG[] = {   0, 255, 255, 255,   0,  51, 255, 255,   0, 170};
//const byte colourTimeB[] = { 255, 255, 255, 180,   0, 255,   0,   0, 255,   0};

// Used to define "test" colours
//                             0    1    2    3    4    5    6    7    8    9
const byte testColoursR[] = { 255,   0,   0, 255, 255,   0, 255,   0,   0,   0};
const byte testColoursG[] = { 255, 255,   0,   0, 255, 255,   0, 128,  32,   0};
const byte testColoursB[] = { 255,   0, 255,   0,   0, 255, 255,   0,   0,   0};

// Used to define "dayOfWeek" colours
//                            0    1    2    3    4    5    6
const byte dayOfWeekR[] = { 255, 255, 204,  51,   0,   0,   0};
const byte dayOfWeekG[] = {   0,   0,   0,   0,  51, 153, 255};
const byte dayOfWeekB[] = {   0, 153, 192, 255, 255, 153,   0};

// These are the "pure" HSV values turned to RGB
// Number                        0    1    2    3    4    5    6    7    8    9
// Degrees                       0   36   72  108  144  180  216  252  288  324
//const byte colourTimeR[] = { 255, 255, 204,  51,   0,   0,   0,  51, 204, 255};
//const byte colourTimeG[] = {   0, 153, 255, 255, 255, 255, 102,   0,   0,   0};
//const byte colourTimeB[] = {   0,   0,   0,   0, 102, 255, 255, 255, 255, 153};
// With these values 2/3 and 6/7 are not easily distinguishable
// 2 is too green, 6 is too blue

#ifdef FEATURE_SEP_LED
const byte LED_ADDR[] = { 0, 1, 2, 3, 5, 6, 7, 8, 10, 11, 12, 13 };
#else
const byte LED_ADDR[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
#endif

// 1 extra element at array index 0 to make the conversion from
// user range 1 - 10 easier
const byte CYCLE_SPEED_MAP[] = { 64, 64, 54, 42, 30, 20, 13, 9, 4, 2, 1 };

extern LEDManager_ &ledManager;