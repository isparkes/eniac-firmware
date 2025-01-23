#pragma once

#include "Arduino.h"
#include "Globals.h"
#include "utilities.h"
#include "Defs.h"
#include "BlankingManager.h"
#include "TransitionManager.h"
#include "DebugManager.h"
#include "CountdownManager.h"

#ifdef FEATURE_TICKER
  #include "QuoteManager.h"
#endif

// -------------------------------------------------------------------------------

#define APPLY_LEAD_0_BLANK true
#define DO_NOT_APPLY_LEAD_0_BLANK false

// -------------------------------------------------------------------------------

#define STUNT_NONE    0
#define STUNT_ACP     0
#define STUNT_SLOTS   0

// -------------------------------------------------------------------------------

#define SLOTS_MODE_MIN                    0
#define SLOTS_MODE_NONE                   0
#define SLOTS_MODE_WIPE                   1
#define SLOTS_MODE_BANG                   2
#define SLOTS_MODE_SCRAMBLE               3
#define SLOTS_MODE_MAX                    3
#define SLOTS_MODE_DEFAULT                2

#define SLOTS_TRIGGER_SECOND              50

// -------------------------------------------------------------------------------

#define ACP_MODE_MIN                    0
#define ACP_MODE_NONE                   0
#define ACP_MODE_1M                     1
#define ACP_MODE_10M                    2
#define ACP_MODE_1H                     3
#define ACP_MODE_MAX                    3
#define ACP_MODE_DEFAULT                2

#define ACP_TICKS_PER_DIGIT             40
#define ACP_TRIGGER_SECOND              15

// -------------------------------------------------------------------------------

// How quickly the scroll works
#define SCROLL_STEPS_DEFAULT 4
#define SCROLL_STEPS_MIN     1
#define SCROLL_STEPS_MAX     8

// -------------------------------------------------------------------------------
// The number of dispay impessions we need to fade by default
// 100 is about 1 second
#define FADE_STEPS_DEFAULT 25
#define FADE_STEPS_MIN     10
#define FADE_STEPS_MAX     60

// -------------------------------------------------------------------------------
#define SEP_MODE_MIN        0
#define SEP_RAILROAD        0
#define SEP_RAILROAD_X      1
#define SEP_BLINK_SLOW      2
#define SEP_BLINK_FAST      3
#define SEP_BLINK_DBL       4
#define SEP_ON              5
#define SEP_OFF             6
#define SEP_AM_PM           7
#define SEP_BLINK_DEFAULT   SEP_RAILROAD
#define SEP_MODE_MAX        7

// -------------------------------------------------------------------------------
// Display mode, set per digit
#define BLANKED  0
#define NORMAL   1
#define BLINK    2

// -------------------------------------------------------------------------------
#define DISPLAY_TIME        0
#define DISPLAY_DATE        1
#define DISPLAY_VALUE       2
#define DISPLAY_COUNTDOWN   3
#define DISPLAY_TICKER      4

// -------------------------------------------------------------------------------

// The mode selection works on a priority scheme
// The lower priority mode will be diplayed if enabled
// ACP and Diags mode override all other modes
// Slots mode means that the secondary display mode overrides the primary mode
// However value mode, if set via an arbitrary value being set, also overrides the primary mode
// Primary mode is shown when none of the other modes is set, i.e. normally

// These modes can be set as primary or secondary: Time / Date / Value / Countdown

// Value mode when set by a REST push can override the primary mode

typedef void (*DebugCallback) (String);

// We display the mode with the highest priority 
//                                                                           Priority |   Stunts    | Blanking
enum outputModes {                          //                                        | Fade Scroll |     
  acpMode,                                  // acp                               1    |   N     N   |    N
  diagsMode,                                // Used during startup test          2    |   N     N   |    N
  secondaryMode,                            // Alternate display slots           3    |   Y     Y   |    Y
  valueMode,                                // Displaying a value                4    |   Y     Y   |    Y
  primaryMode,                              // Primary (normal time) mode        5    |   Y     Y   |    Y
};

typedef enum {
    digit0 = 0,
    digit1 = 1,
    digit2 = 2,
    digit3 = 3,
    digit4 = 4,
    digit5 = 5,
    digit6 = 6,
    digit7 = 7,
    digit8 = 8,
    digit9 = 9
} clock_digit;

class OutputManager_ {
  friend class Transition;
  private:
    OutputManager_() = default; // Make constructor private

  public:
    static OutputManager_ &getInstance(); // Accessor for singleton instance

    OutputManager_(const OutputManager_ &) = delete; // no copying
    OutputManager_ &operator=(const OutputManager_ &) = delete;

  public:
    // Do a display impression
    void outputDisplay();

    // Special display types
    void loadNumberArraySameValue(byte value);
    void loadNumberArrayBurn(byte value);
    void incrementNumberArray();      // Used in "scramble" stunt

    outputModes getOutputMode();
    void setOutputMode(outputModes newMode);

    void updateOncePerSecond();

    byte getNextACPMode();
    String getNextACPModeName();
    void setACPMode(byte newMode);
    void setNextACPMode();

    byte getNextSlotsMode();
    String getNextSlotsModeName();
    void setSlotsMode(byte newMode);
    void setNextSlotsMode();

    void setBlankingStatusTubes(bool newStatus);
    void setBlankingStatusTowers(bool newStatus);

    // Blank tubes until the next display change;
    void forceBlanking();

    void loadNumberArrayPrimary();
    void loadNumberArraySecondary();
    void setArbitraryValue(unsigned int value);
    void setArbitraryValueDisplayTime(unsigned int value);

    byte getCurrentDisplayDigitValue(byte digit);

    #ifdef OTM_EXTENDED_DEBUG
    void dumpNumberArrayValues();
    #endif
  private:
    // Anti Cathode Poisoning management
    byte _acpOffset = 0;
    byte _acpTick = 0;

    // Separators and indicator LEDs
    bool _sep1State;
    bool _sep2State;
    bool _sep3State;
    bool _sep4State;

    // Control blanking
    bool _blankTubes;
    bool _blankSeparators;
    bool _blankTubesTemp;

    clock_digit numberArray[DIGIT_COUNT]     = {digit0, digit0, digit0, digit0, digit0, digit0};
    clock_digit currNumberArray[DIGIT_COUNT] = {digit0, digit0, digit0, digit0, digit0, digit0};
    byte displayType[DIGIT_COUNT]     = {NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL};
    int fadeState                     = 0;
    byte scrollCounter[DIGIT_COUNT]   = {0, 0, 0, 0, 0, 0};
    byte valueDisplayTime             = 0;
    byte valueToShow[3]               = {0, 0, 0};
    byte valueDisplayType[3]          = {0x33, 0x33, 0x33}; // All normal by default

    outputModes _outputMode;

    // Value mode
    unsigned int _arbitraryValue;
    unsigned long _arbitraryValueEndTime;

    // Main display types - accesible via the setPrimary/Secondary methods
    void loadNumberArrayTime();
    void loadNumberArrayDate();
    void loadNumberArrayValue();
    void loadNumberArrayTicker();

    void loadNumberArrayIntegerValue(unsigned int value);
    void loadNumberArrayInternal(byte mode);

    clock_digit convertToDigit(int value);
    void allNormal(bool leadingBlank);
    void allBlanked();
    void applyBlanking();

    void triggerStunts();
    void processStunts();
    uint32_t decodeFromNumberArray(byte valueToDecodeTens, byte valueToDecodeUnits, bool blankTens, bool bankUnits, bool blankSeparators, bool bl1, bool bl2, bool led1, bool led2);
    void setCurrentTransition();
    void processSeparators();
};

extern OutputManager_ &outputManager;
