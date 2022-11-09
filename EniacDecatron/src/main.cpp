#include <Arduino.h>

//**********************************************************************************
//* Code for a Dual Decatron Spinner millicycles display                           *
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
int indexMark1 = 0;  // The current index mark we have detected (0..29)
int tdc1 = 0;        // The required Top Dead Center "12 o'clock" (0..29)

int digitStep2 = 0;
int phaseStep2 = 0;
int currentPos2 = 0; // The current position we are at (0..29)
int indexMark2 = 0;
int tdc2 = 0;

bool g1Mark;

volatile int millisInSecond = 0;

// ------------------ Usage statistics -----------------

int impressionsPerSec = 0;
int lastImpressionsPerSec = 0;

// ----------------- Real time clock -------------------

byte useRTC = false;  // true if we detect an RTC
boolean onceHadAnRTC = false;

// ------------- Time management variables -------------

unsigned long nowMillis = 0;
unsigned long lastCheckMillis = 0;
unsigned long lastSecMillis = nowMillis;
int lastSecond = second();
boolean secondsChanged = false;
boolean triggeredThisSec = false;

// --------------------- Blanking ----------------------

boolean blanked = false;
byte blankSuppressStep = 0;    // The press we are on: 1 press = suppress for 1 min, 2 press = 1 hour, 3 = 1 day
unsigned long blankSuppressedMillis = 0;   // The end time of the blanking, 0 if we are not suppressed
unsigned long blankSuppressedSelectionTimoutMillis = 0;   // Used for determining the end of the blanking period selection timeout

// --------------------- Misc ----------------------

bool debugVal = DEBUG;

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
  debugManager.debugMsg("G1B");
  phaseStep1++;

  if (phaseStep1 > 2) {
    phaseStep1 = 0;
    digitStep1++;
    if (digitStep1 > 9) {
      digitStep1 = 0;
    }
  }

  currentPos1 = phaseStep1 + digitStep1 * 3;
  debugManager.debugMsg("G1 at " + String(currentPos1));

  G_step1(phaseStep1);
}

// ************************************************************
// step forward on Decatron 2
// ************************************************************
void G2StepBackwards() {
  debugManager.debugMsg("G2B");
  phaseStep2++;

  if (phaseStep2 > 2) {
    phaseStep2 = 0;
    digitStep2++;
    if (digitStep2 > 9) {
      digitStep2 = 0;
    }
  }

  currentPos2 = phaseStep2 + digitStep2 * 3;
  debugManager.debugMsg("G2 at " + String(currentPos2));

  G_step2(phaseStep2);
}

// ************************************************************
// step backward on Decatron 1
// ************************************************************
void G1StepForwards() {
  debugManager.debugMsg("G1F");
  phaseStep1--;

  if (phaseStep1 < 0) {
    phaseStep1 = 2;
    digitStep1--;
    if (digitStep1 < 0) {
      digitStep1 = 9;
    }
  }

  currentPos1 = phaseStep1 + digitStep1 * 3;

  if (currentPos1 == indexMark1) {
    g1Mark = true;
  } else {
    g1Mark = false;
  }

  debugManager.debugMsg("G1 at " + String(currentPos1));

  G_step1(phaseStep1);
}

// ************************************************************
// step backward on Decatron 2
// ************************************************************
void G2StepForwards() {
  debugManager.debugMsg("G2F");
  phaseStep2--;

  if (phaseStep2 < 0) {
    phaseStep2 = 2;
    digitStep2--;
    if (digitStep2 < 0) {
      digitStep2 = 9;
    }
  }

  currentPos2 = phaseStep2 + digitStep2 * 3;
  debugManager.debugMsg("G2 at " + String(currentPos2));

  G_step2(phaseStep2);
}

// ************************************************************
// Set Top Dead Centre on Decatron 1
// ************************************************************
void setTDC1() {
  if (digitalRead(Index1) == LOW) {
    indexMark1 = currentPos1;

    if (currentPos1 != tdc1) {
      G1StepForwards();
    }
  }
}

// ************************************************************
// Set Top Dead Centre on Decatron 1
// ************************************************************
void setTDC2() {
  if (digitalRead(Index2) == LOW) {
    indexMark2 = currentPos2;

    if (currentPos2 != tdc2) {
      G2StepForwards();
    }
  }
}

// ************************************************************
// Find Top Dead Centre on Decatron 1
// ************************************************************
bool getTDC1() {
  if (digitalRead(Index1) == LOW) {
    tdc1 = currentPos1;
    debugManager.debugMsg("G1 found index mark at " + String(tdc1));
    return true;
  }

  return false;
}

// ************************************************************
// Find Top Dead Centre on Decatron 1
// ************************************************************
bool getTDC2() {
  if (digitalRead(Index2) == LOW) {
    tdc2 = currentPos2;
    debugManager.debugMsg("G2 found index mark at " + String(tdc2));
    return true;
  }

  return false;
}

// ************************************************************
// Called once per second
// ************************************************************
void performOncePerSecondProcessing() {
  // Store the current value and reset
  lastImpressionsPerSec = impressionsPerSec;
  impressionsPerSec = 0;

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

  bool tdc1Found = false;
  bool tdc2Found = false;

  while (!tdc1Found) { 
    debugManager.debugMsg("Stepping G1 to find index");
    G1StepForwards();
    tdc1Found = getTDC1();
  }

  // Add another count to get to the real top
  G1StepForwards();
  G1StepForwards();

  while (!tdc2Found) { 
    debugManager.debugMsg("Stepping G2 to find index");
    G2StepForwards();
    tdc2Found = getTDC2();
  }

  // Add another count to get to the real top
  G2StepForwards();
  G2StepForwards();

  delay(5000);

  debugManager.debugMsg("Startup done");
}

// ----------------------------------------------------------------------------------------------------
// -----------------------------------------------  Loop  ---------------------------------------------
// ----------------------------------------------------------------------------------------------------
void loop() {
  nowMillis = millis();

  // shows us how fast the inner loop is running
  impressionsPerSec++;

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

  if (blanked) {
    digitalWrite(HVEnable, false);
  } else {
    digitalWrite(HVEnable, true);
  }

  G1StepForwards();

  if (g1Mark) {
    G2StepForwards();
  }
  
  delay(32);
}

