#pragma once

#include "Arduino.h"
#include "Globals.h"
#include "utilities.h"
#include "Defs.h"
#include "BlankingManager.h"
#include "TransitionManager.h"
#include "DebugManager.h"
#include "CountdownManager.h"

#define DIGIT_PWM_CHANNEL  0

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
#define SLOTS_MODE_MAX                    2
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

#define ACP_TICKS_PER_DIGIT             25
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
#define DISPLAY_TYPE_TIME   0
#define DISPLAY_TYPE_DATE   1

// -------------------------------------------------------------------------------
// Display mode, set per digit
#define BLANKED  0
#define NORMAL   1
#define BLINK    2

// -------------------------------------------------------------------------------

typedef void (*DebugCallback) (String);

enum outputModes {                          //                              ACP Allowed   Slots Allowed   Fade   Scroll
  diagsMode,                                // Used during startup test           N             N          N       N
  primaryDisplayMode,                       // normal time mode                   Y             Y          Y       Y
  secondaryDisplayMode,                     // dates slots                        N             -          Y       Y
  valueMode,                                // we are displaying a value          N             N          Y       Y
  acpMode                                   // acp                                -             N          N       N
};

class OutputManager_ {
  private:
    OutputManager_(); // Make constructor private

  public:
    static OutputManager_ &getInstance(); // Accessor for singleton instance

    OutputManager_(const OutputManager_ &) = delete; // no copying
    OutputManager_ &operator=(const OutputManager_ &) = delete;

  public:
    // These load the configured primary and secondary display modes
    void loadNumberArrayPrimary();
    void loadNumberArraySecondary();

    // These are the native modes we can use
    void loadNumberArrayTime();
    void loadNumberArrayDate();
    void loadNumberArraySameValue(byte value);
    void loadNumberArrayBurn(byte value);
    void loadNumberArrayValue(unsigned int value);

    void allNormal(bool leadingBlank);
    void allBlanked();

    void outputDisplay();

    void applyBlanking();
    void triggerStunts();
    outputModes getOutputMode();
    void setOutputMode(outputModes newMode);

    void setOutputModeOncePerSecond();

    byte getNextACPMode();
    String getNextACPModeName();
    void setACPMode(byte newMode);
    void setNextACPMode();
    byte getNextSlotsMode();
    String getNextSlotsModeName();
    void setSlotsMode(byte newMode);
    void setNextSlotsMode();

    // Digit PWM Channel
    void setUpDigitPWM();
    void setDigitPWM(int pwmValue);
  private:
    int _acpOffset = 0;
    int _acpTick = 0;

    // Separators and indicator LEDs
    bool _led1State;
    bool _led2State;
    bool _led3State;
    bool _led4State;
    bool _indLed1;
    bool _indLed2;

    outputModes _outputMode;

    void processStunts();
    uint32_t decodeFromNumberArray(byte valueToDecodeTens, byte valueToDecodeUnits, bool blankTens, bool bankUnits, bool blankSeparators, bool bl1, bool bl2, bool led1, bool led2);
    void setCurrentTransition();
    void processSeparators();
};

extern OutputManager_ &outputManager;
