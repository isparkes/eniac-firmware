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

volatile int digitStep1 = 0;  // Cathode we are on (0..2)
volatile int phaseStep1 = 0;  // Step inside of the cathode (0..9)
volatile int currentPos1 = 0; // The current position we are at (0..29)
volatile int indexMark1 = -1; // The current index mark we have detected (0..29)
volatile int tdc1 = 0;        // The required Top Dead Center "12 o'clock" (0..29)

volatile int digitStep2 = 0;
volatile int phaseStep2 = 0;
volatile int currentPos2 = 0;
volatile int indexMark2 = -1;
volatile int tdc2 = 0;

volatile bool g1Mark;
volatile bool g2Mark;

volatile int millisInSecond = 0;

// ------------- Time management variables -------------

volatile unsigned long nowMillis = 0;
unsigned long lastCheckMillis = 0;
unsigned long lastSecMillis = nowMillis;
unsigned long lastPulseMillis = 0;
int lastSecond = second();
boolean secondsChanged = false;
boolean triggeredThisSec = false;

// --------------------- Blanking ----------------------

bool blanked = false;
bool blankedLastState = false;

// --------------------- Protocol ----------------------

unsigned long lastPulse = 0;
unsigned long pulseLength = 0;
bool inPulse = false;
int shorts = 0;
int longs = 0;

// --------------------- Misc ----------------------

bool debugVal = DEBUG;

volatile uint32_t muxCount = INT_MUX_COUNTS_OFF;

// -------------------------------------------------

// ----------------------------------------------------------------------------------------------------
// -----------------------------------------  Utility functions  --------------------------------------
// ----------------------------------------------------------------------------------------------------

// ************************************************************
// Manage HV
// ************************************************************
void enableHV(bool state) {
  if (state) {
    debugManager.debugMsg("HV ON");
    digitalWrite(HVEnable, HIGH);
  } else {
    debugManager.debugMsg("HV OFF");
    digitalWrite(HVEnable, LOW);
  }
}

// ************************************************************
// Perform a step on Decatron 1
// ************************************************************
IRAM_ATTR void G_step1(int CINT)
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
IRAM_ATTR void G_step2(int CINT)
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
IRAM_ATTR void G1StepBackwards() {
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
IRAM_ATTR void G2StepBackwards() {
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
IRAM_ATTR void G1StepForwards() {
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
IRAM_ATTR void G2StepForwards() {
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
  indexMark2 = -1;

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

  // for (int i = 0 ; i < 90 ; i++) {
  //   G2StepForwards();
  //   delay(10);
  //   G1StepBackwards();
  //   delay(10);
  // }

  // delay(1000);
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
// Called once per second
// ************************************************************
void performOncePerSecondProcessing() {
  // ------------------------------------
  debugManager.debugMsg("Longs: " + String(longs) + ", Shorts: " + String(shorts));
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

// ----------------------------------------------------------------------------------------------------
// ----------------------------------------  Interrupt handlers  --------------------------------------
// ----------------------------------------------------------------------------------------------------

// ************************************************************
// Slow Decatron Index Handling! (Fast Decatron runs freely)
// Detect the falling edge of the Index pin and get the index
// value - adjust it to allow the display to rotate
// ************************************************************
IRAM_ATTR void handleIndexMarkTrigger2() {
  indexMark2 = digitStep2;
}

// ************************************************************
// Slow Decatron Index Handling! (Fast Decatron runs freely)
// Interrupt routine for scheduled interrupts
// Spin through each of the pins on the decatron, using a short
// interrupt time for "off" pins (not really off, but quite dim)
// and longer dwell times for the lit pins.
// ************************************************************
IRAM_ATTR void displayUpdateTimer() {
  uint32_t delayCount = muxCount;

  G2StepForwards();

  timer1_write(delayCount);
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

  enableHV(true);

  debugManager.debugMsg("Find Index Mark");
  findIndexMarks();

  debugManager.debugMsg("Starting GPIO interrupt handler");

  pinMode(inputPin1, INPUT_PULLUP);
//   attachInterrupt(digitalPinToInterrupt(inputPin1), handleButtonInterrupt, CHANGE);  

  debugManager.debugMsg("Starting display interrupt handler for scanning");

  attachInterrupt(digitalPinToInterrupt(Index2), handleIndexMarkTrigger2, FALLING);

  timer1_attachInterrupt(displayUpdateTimer);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
  timer1_write(muxCount);

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

  if ((nowMillis - lastPulseMillis) > 33) {
    G1StepForwards();
    lastPulseMillis = nowMillis;
  }

  blankedLastState = blanked;

  // Manage the state based on the interrupt readings
  // If we last received a pulse more than a second ago, then blank
  blanked = ((nowMillis - lastPulse) > 10100);

  if (blanked != blankedLastState) {
    if (blanked) {
      digitalWrite(HVEnable, false);
    } else {
      digitalWrite(HVEnable, true);
      findIndexMarks();
    }
  }

  if (digitalRead(inputPin1) == LOW) {
    if (inPulse == false){
      inPulse = true;
      lastPulse = nowMillis;
    }
  } else {
    if (inPulse == true) {
      inPulse = false;
      unsigned long pulseLength = nowMillis - lastPulse;
      if (pulseLength > 100) {
        longs++;
      } else {
        shorts++;
      }
    }
  }

  delay(1);
}
