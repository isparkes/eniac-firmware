#pragma once

#include "Arduino.h"
#include "defs.h"
#include "globals.h"
#include "OutputManager.h"

#define SLOTS_MODE_WIPE_WIPE 0
#define SLOTS_MODE_BANG_BANG 1

class Transition
{
  public:
    Transition(int, int, int, int);
    void start(unsigned long);
    boolean isMessageOnDisplay(unsigned long);
    boolean runEffect(unsigned long, boolean blankLeading);
    void updateRegularDisplaySeconds(byte secondUpdate);
    
  private:
    int _effectInDuration;
    int _effectOutDuration;
    int _holdDuration;
    int _selectedEffect;
    int _digit;
    unsigned long _started;
    unsigned long _end;
    byte _regularDisplay[DIGIT_COUNT] = {0, 0, 0, 0, 0, 0};
    byte _alternateDisplay[DIGIT_COUNT] = {0, 0, 0, 0, 0, 0};
    byte _savedDisplayType[DIGIT_COUNT] = {0, 0, 0, 0, 0, 0};
    
    boolean wipeInWipeOut(unsigned long, boolean blankLeading);
    boolean bangInBangOut(unsigned long);
    unsigned long getEnd();
};
