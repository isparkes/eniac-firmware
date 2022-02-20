#pragma once

#include "Arduino.h"
#include "globals.h"
#include "utilities.h"
#include "defs.h"
#include "BlankingManager.h"
#include "TransitionManager.h"

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

typedef void (*DebugCallback) (String);

enum outputModes {
  timeMode,                                 // norml time mode
  slotsMode,                                // dates slots
  acpMode                                   // acp
};

class OutputManager_ {
  private:
    OutputManager_() = default; // Make constructor private

  public:
    static OutputManager_ &getInstance(); // Accessor for singleton instance

    OutputManager_(const OutputManager_ &) = delete; // no copying
    OutputManager_ &operator=(const OutputManager_ &) = delete;

  public:
    void loadNumberArrayTime();
    void loadNumberArrayDate();
    void loadNumberArraySameValue(byte value);
    void loadNumberArrayBurn(byte value);

    void allNormal(bool leadingBlank);
    void allBlanked();

    void outputDisplay();

    void applyBlanking();
    void triggerStunts();
    outputModes getOutputMode();

    void setDebugOutput(bool newDebug);
    
    // callbacks
    void setDebugCallback(DebugCallback dbcb);
  private:
    int _acpOffset = 0;
    int _acpTick = 0;

    outputModes _outputMode;

    void processStunts();
    uint32_t decodeFromNumberArray(byte valueToDecodeTens, byte valueToDecodeUnits, bool blankTens, bool blankUnits, bool bl1, bool bl2, bool led1, bool led2);
    void setCurrentTransition();

    DebugCallback _dbcb;
    bool _debug = false;

    void debugMsg(String message);                        // print a debug message to the callback
};

extern OutputManager_ &outputManager;
