#include <Arduino.h>

//**********************************************************************************
//* Code for a Dual Decatron Spinner slave display                                 *
//*                                                                                *
//*  nixie@protonmail.ch                                                           *
//*                                                                                *
//* Board: Lolin(Wemos) D1 R2 & Mini                                               *
//* CPU-Frequency: 80MHz                                                           *
//* Flash Size: 4MB (FS: 2MB, OTA:~1019KB)                                         *
//*                                                                                *
//**********************************************************************************
//**********************************************************************************

#include <TimeLib.h>            // http://playground.arduino.cc/code/time (Margolis 1.5.0) // https://github.com/michaelmargolis/arduino_time

#include "defs.h"
#include "DebugManager.h"

#define DEBUG     true

// ------------------ Decatron Control ----------------

int digitStep1 = 0;  // Cathode we are on (0..2)
int phaseStep1 = 0;  // Step inside of the cathode (0..9)
int currentPos1 = 0; // The current position we are at (0..29)
int indexMark1 = -1; // The current index mark we have detected (0..29)
int tdc1 = 0;        // The required Top Dead Center "12 o'clock" (0..29)
int expPos1 = 0;     // The expected position we are aiming for

int digitStep2 = 0;
int phaseStep2 = 0;
int currentPos2 = 0;
int indexMark2 = -1;
int tdc2 = 0;
int expPos2 = 0;

bool g1Mark;
bool g2Mark;

volatile int millisInSecond = 0;

// ------------- Time management variables -------------

unsigned long nowMillis = 0;
unsigned long lastCheckMillis = 0;
unsigned long lastSecMillis = nowMillis;
unsigned long hundredthsMillis = 0;
int lastSecond = second();
boolean secondsChanged = false;
boolean triggeredThisSec = false;

// --------------------- Blanking ----------------------

boolean blanked = false;

// --------------------- Protocol ----------------------

unsigned long lastInterrupt = 0;
unsigned long lastHigh = 0;
unsigned long lastLow = 0;
unsigned long pulseLength = 0;

// --------------------- Misc ----------------------

bool debugVal = DEBUG;

const int interruptPin = 0; //GPIO 0 / D3 (Button)

// -------------------------------------------------

void enableHV() {
  debugManager.debugMsg("HV ON");
  digitalWrite(HVEnable, HIGH);
}

void disableHV() {
  debugManager.debugMsg("HV OFF");
  digitalWrite(HVEnable, LOW);
}

// ************************************************************
// Perform a step on Decatron 1
// ************************************************************
void G_step1(int CINT)
{
  if (CINT == 0)
  {
    digitalWrite(Guide1_1, LOW);
    digitalWrite(Guide2_1, LOW);
  }
  if (CINT == 1)
  {
    digitalWrite(Guide1_1, HIGH);
    digitalWrite(Guide2_1, LOW);
  }
  if (CINT == 2)
  {
    digitalWrite(Guide1_1, LOW);
    digitalWrite(Guide2_1, HIGH);
  }
}

// ************************************************************
// Perform a step on Decatron 1
// ************************************************************
void G_step2(int CINT)
{
  if (CINT == 0)
  {
    digitalWrite(Guide1_2, LOW);
    digitalWrite(Guide2_2, LOW);
  }
  if (CINT == 1)
  {
    digitalWrite(Guide1_2, HIGH);
    digitalWrite(Guide2_2, LOW);
  }
  if (CINT == 2)
  {
    digitalWrite(Guide1_2, LOW);
    digitalWrite(Guide2_2, HIGH);
  }
}

// ************************************************************
// step forward on Decatron 1
// ************************************************************
void G1StepBackwards() {
//  debugManager.debugMsg("G1B");
  phaseStep1++;

  if (phaseStep1 > 2) {
    phaseStep1 = 0;
    digitStep1++;
    if (digitStep1 > 9) {
      digitStep1 = 0;
    }
  }

  currentPos1 = phaseStep1 + digitStep1 * 3;
//  debugManager.debugMsg("G1 at " + String(currentPos1));

  G_step1(phaseStep1);
}

// ************************************************************
// step forward on Decatron 2
// ************************************************************
void G2StepBackwards() {
//  debugManager.debugMsg("G2B");
  phaseStep2++;

  if (phaseStep2 > 2) {
    phaseStep2 = 0;
    digitStep2++;
    if (digitStep2 > 9) {
      digitStep2 = 0;
    }
  }

  currentPos2 = phaseStep2 + digitStep2 * 3;
//  debugManager.debugMsg("G2 at " + String(currentPos2));

  G_step2(phaseStep2);
}

// ************************************************************
// step backward on Decatron 1
// ************************************************************
void G1StepForwards() {
//  debugManager.debugMsg("G1F");
  phaseStep1--;

  if (phaseStep1 < 0) {
    phaseStep1 = 2;
    digitStep1--;
    if (digitStep1 < 0) {
      digitStep1 = 9;
    }
  }

  currentPos1 = phaseStep1 + digitStep1 * 3;

  g1Mark = (currentPos1 == indexMark1);

//  debugManager.debugMsg("G1 at " + String(currentPos1));

  G_step1(phaseStep1);
}

// ************************************************************
// step backward on Decatron 2
// ************************************************************
void G2StepForwards() {
//  debugManager.debugMsg("G2F");
  phaseStep2--;

  if (phaseStep2 < 0) {
    phaseStep2 = 2;
    digitStep2--;
    if (digitStep2 < 0) {
      digitStep2 = 9;
    }
  }

  g2Mark =  (currentPos2 == indexMark2);

  currentPos2 = phaseStep2 + digitStep2 * 3;
//  debugManager.debugMsg("G2 at " + String(currentPos2));

  G_step2(phaseStep2);
}

// ************************************************************
// Find the index mark
// ************************************************************
void findIndexMarks() {
  indexMark1 = -1;
  indexMark2 = -1;

  while ((indexMark1 < 0) | (indexMark2 < 0)) {
    if(indexMark1 < 0) {
      G1StepForwards();
      delay(10);
      if (digitalRead(Index1) == LOW) {
        indexMark1 = currentPos1;
      }
    }

    if(indexMark2 < 0) {
      G2StepForwards();
      delay(10);
      if (digitalRead(Index2) == LOW) {
        indexMark2 = currentPos2;
      }
    }
  }

  indexMark1 = -1;
  G1StepBackwards();
  delay(10);
  G1StepBackwards();
  delay(10);
  G1StepBackwards();
  delay(10);

  indexMark2 = -1;
  G2StepBackwards();
  delay(10);
  G2StepBackwards();
  delay(10);
  G2StepBackwards();
  delay(10);

//  delay(1000);

  while ((indexMark1 < 0) | (indexMark2 < 0)) {
    if(indexMark1 < 0) {
      G1StepBackwards();
      delay(10);
      if (digitalRead(Index1) == LOW) {
        indexMark1 = currentPos1;
      }
    }

    if(indexMark2 < 0) {
      G2StepBackwards();
      delay(10);
      if (digitalRead(Index2) == LOW) {
        indexMark2 = currentPos2;
      }
    }
  }

//  delay(1000);

  // More steps to get to the top
  G1StepForwards();
  delay(10);
  G1StepForwards();
  delay(10);

  G2StepForwards();
  delay(10);
  G2StepForwards();
  delay(10);
  G2StepForwards();
  delay(10);

  tdc1 = currentPos1;
  tdc2 = currentPos2;

  delay(1000);
}

// ************************************************************
// Called once per second
// ************************************************************
void performOncePerSecondProcessing() {
  // ------------------------------------
}

// ************************************************************
// Called once per minute
// ************************************************************
void performOncePerMinuteProcessing() {
  debugManager.debugMsg("---> OncePerMinuteProcessing");
}

// ************************************************************
// Called once per hour
// ************************************************************
void performOncePerHourProcessing() {
  debugManager.debugMsg("---> OncePerHourProcessing");
}

// ************************************************************
// Called once per day
// ************************************************************
void performOncePerDayProcessing() {
  debugManager.debugMsg("---> OncePerDayProcessing");
}

// ************************************************************
// Local debug routine
// ************************************************************
void debugMsgLocal(String message) {
  debugManager.debugMsg(message);
}

// ************************************************************
// See if we have enough flash space for OTA
// ************************************************************
boolean getOTAvailable() {
  return ESP.getSketchSize() * 2 < ESP.getFlashChipSize();
}

// ************************************************************
// Interrupt handler
// ************************************************************
void handleInterrupt() {
  lastInterrupt = nowMillis;

  bool pinState = digitalRead(interruptPin);

  if (pinState) {
    lastHigh = nowMillis;
  } else {
    lastLow = nowMillis;
    pulseLength = lastLow - lastHigh;
  }


}

// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------  Set up  --------------------------------------------
// ----------------------------------------------------------------------------------------------------
void setup() {
  debugManager.setUp(debugVal);

  pinMode(Guide1_1, OUTPUT);
  pinMode(Guide2_1, OUTPUT);
  pinMode(Index1, INPUT);

  pinMode(Guide1_2, OUTPUT);
  pinMode(Guide2_2, OUTPUT);
  pinMode(Index2, INPUT);

  pinMode(HVEnable, OUTPUT);

  debugManager.debugMsg("Started");

  // initialise the internal time (in case we don't find the time provider)
  nowMillis = millis();

  enableHV();

  debugManager.debugMsg("Find Index Mark");
  findIndexMarks();

  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, CHANGE);  

  debugManager.debugMsg("Startup done");
}

// ----------------------------------------------------------------------------------------------------
// -----------------------------------------------  Loop  ---------------------------------------------
// ----------------------------------------------------------------------------------------------------
void loop() {
  nowMillis = millis();

  // -------------------------------------------------------------------------------

  if (lastSecond != second()) {
    lastSecond = second();
    lastSecMillis = nowMillis;
    secondsChanged = true;
    performOncePerSecondProcessing();

    if ((second() == 0) && (!triggeredThisSec)) {
      if ((minute() == 0)) {
        if (hour() == 0) {
          performOncePerDayProcessing();
        }
        performOncePerHourProcessing();
      }
      performOncePerMinuteProcessing();
    }

    // Make sure we don't call multiple times
    triggeredThisSec = true;

    if ((second() > 0) && triggeredThisSec) {
      triggeredThisSec = false;
    }
  }

  millisInSecond = nowMillis - lastSecMillis;

  // Manage the state based on the interrupt readings
  // If we last received a pulse more than a second ago, then blank
  blanked = ((nowMillis - lastInterrupt) > 1100);

  if (blanked) {
    digitalWrite(HVEnable, false);
  } else {
    digitalWrite(HVEnable, true);
  }

  // Spin the fast one
  G2StepForwards();

  if (g2Mark) {
    G1StepForwards();
  }

  // Align the G1 TDC with the second
  if (pulseLength > 100) {
    while (!g1Mark)
    {
      G1StepForwards();
      delay(1);
    }
  }

  delay(1);
}
