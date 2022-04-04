/**
 * A class that displays a message by wiping it into and out of the display
 *
 * Thanks Judge & Ty!
 */

#include "TransitionManager.h"

Transition transitionWipe(800, 700, 2800, SLOTS_MODE_WIPE_WIPE, "Wipe");  // Wipe In / Wipe Out (DA2000-Transition.h)  
Transition transitionBang(400, 400, 3200, SLOTS_MODE_BANG_BANG, "Bang");  // Bang In / Bang Out (DA2000-Transition.h)  
Transition transitionDummy(0,    0,    0, SLOTS_MODE_NONE,      "None");  // Dummy transition for null pointer prevention
Transition *activeTransition = &transitionDummy;                  // Pointer to selected transition object

Transition::Transition(int effectInDuration, int effectOutDuration, int holdDuration, int selectedEffect, String name) {
  _effectInDuration = effectInDuration;
  _effectOutDuration = effectOutDuration;
  _holdDuration = holdDuration;
  _selectedEffect = selectedEffect;
  _started = 0;
  _end = 0;
  _name = name;
}

void Transition::start(unsigned long now) {
  if (_end < now) {
    // save the target display
    outputManager.loadNumberArrayDate();
    for (int idx = 0; idx < DIGIT_COUNT ; idx++) {
      _alternateDisplay[idx] = numberArray[idx];
    }

    // save the current version of the normal display
    outputManager.loadNumberArrayTime();
    for (int idx = 0; idx < DIGIT_COUNT ; idx++) {
      _regularDisplay[idx] = numberArray[idx];
      _savedDisplayType[idx] = displayType[idx];
    }
    _started = now;
    _end = getEnd();
  }
  // else we are already running!
}

boolean Transition::runEffect(unsigned long now, boolean blankLeading) {
  switch (_selectedEffect) {
    case SLOTS_MODE_WIPE_WIPE:
      return wipeInWipeOut(now, blankLeading);
      break;
    case SLOTS_MODE_BANG_BANG:
      return bangInBangOut(now);
      break;
    default:
      return false;
      break;
  }
}

boolean Transition::isMessageOnDisplay(unsigned long now)
{
  return (now < _end);
}

boolean Transition::wipeInWipeOut(unsigned long now, boolean blankLeading)
{
  if (now < _end) {
    int msCount = now - _started;
    // Wipe In blanking
    if (msCount < _effectInDuration) {
      _digit = msCount * (DIGIT_COUNT + 1) / _effectInDuration;
      if (_digit > 0)
        displayType[_digit-1] = BLANKED;
    }
    // Wipe In date values
    else if (msCount < _effectInDuration * 2) {
      _digit = (msCount - _effectInDuration) * DIGIT_COUNT / _effectInDuration;
      numberArray[_digit] = _alternateDisplay[_digit];
      displayType[_digit] =  NORMAL;
    }
    // Hold date display
    else if (msCount < _effectInDuration * 2 + _holdDuration) {
      outputManager.loadNumberArrayDate();
    }
    // Wipe Out blanking
    else if (msCount < _effectInDuration * 2 + _holdDuration + _effectOutDuration) {
      _digit = (msCount - _holdDuration - _effectInDuration * 2) * DIGIT_COUNT / _effectOutDuration;
      displayType[_digit] = BLANKED;
    }
    // Wipe Out to time values
    else if (msCount < _effectInDuration * 2 + _holdDuration + _effectOutDuration * 2) {
      _digit = (msCount - _holdDuration - _effectInDuration * 2 - _effectOutDuration) * DIGIT_COUNT / _effectOutDuration;
      numberArray[_digit] = _regularDisplay[_digit];
      if (!blankLeading || _digit != 0 || _regularDisplay[_digit] != 0)
        displayType[_digit] = NORMAL;
    }
    // We now return you to your regularly scheduled program
    else {
      outputManager.loadNumberArrayTime();
      outputManager.allNormal(APPLY_LEAD_0_BLANK);
      _end = 0;
      return false;   // We're done running
   }
    return true;  // we are still running
  }
  return false;   // We aren't running
}

boolean Transition::bangInBangOut(unsigned long now)
{
  if (now < _end) {
    int msCount = now - _started;
    // Bang In blanking
    if (msCount < _effectInDuration) {
      outputManager.allBlanked();
    }
    // Bang In date values
    else if (msCount < _effectInDuration * 2) {
      outputManager.loadNumberArrayDate();
      outputManager. allNormal(DO_NOT_APPLY_LEAD_0_BLANK);
    }
    // Hold date display
    else if (msCount < _effectInDuration * 2 + _holdDuration) {
      outputManager.loadNumberArrayDate();
      outputManager. allNormal(DO_NOT_APPLY_LEAD_0_BLANK);
    }
    // Bang Out blanking
    else if (msCount < _effectInDuration * 2 + _holdDuration + _effectOutDuration) {
      outputManager.allBlanked();
    }
    // Bang Out to time values
    else if (msCount < _effectInDuration * 2 + _holdDuration + _effectOutDuration * 2) {
      outputManager.loadNumberArrayTime();
      outputManager. allNormal(APPLY_LEAD_0_BLANK);
    }
    // We now return you to your regularly scheduled program
    else {
      outputManager.loadNumberArrayTime();
      outputManager. allNormal(APPLY_LEAD_0_BLANK);
      _end = 0;
      return false;   // We're done running
   }
    return true;  // we are still running
  }
  return false;   // We aren't running
}

unsigned long Transition::getEnd() {
  return _started + _effectInDuration * 2 + _holdDuration + _effectOutDuration * 2;
}

// Update the seconds in the internal buffer only needed with 6 digit displays
void Transition::updateRegularDisplaySeconds(byte secondUpdate) {
  if (DIGIT_COUNT == 6) {
    _regularDisplay[S10] = secondUpdate / 10;
    _regularDisplay[S1] = secondUpdate % 10;
  }
}
