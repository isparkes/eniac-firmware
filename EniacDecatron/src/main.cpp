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

#include "defs.h"
#include "DebugManager.h"

#define DEBUG     true
#define DEBUG_OFF false

// --------------------------------- Protocol ------------------------------------
// Serial communication once per second (UART0, 115200 baud)
//   Byte 0: Start byte (0xAA)
//   Byte 1: Hours   (0-23)
//   Byte 2: Minutes (0-59)
//   Byte 3: Seconds (0-59)
//   Byte 4: Control
//     Bit 0:   Blanked (1 = display is blanked)
//     Bits 1-4: Primary display mode (cc->pMode)
// -------------------------------------------------------------------------------

#define SERIAL_START_BYTE             0xAA
#define SERIAL_PACKET_SIZE            4

// Control byte bit masks
#define DECATRON_CTRL_BLANKED         0x01  // Bit 0: display is blanked
#define DECATRON_CTRL_MODE_SHIFT      1     // Bits 1-4: primary display mode

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

byte receivedHour = 0;
byte receivedMinute = 0;
byte receivedSecond = 0;
byte receivedControl = 0;
bool receivedData = false;

// ------------- Time management variables -------------

volatile unsigned long nowMillis = 0;
unsigned long lastSecMillis = 0;

// --------------------- Blanking ----------------------

boolean blanked = false;

// --------------------- I2C Received Data ----------------------

volatile uint8_t rxHour    = 0;
volatile uint8_t rxMinute  = 0;
volatile uint8_t rxSecond  = 0;
volatile uint8_t rxControl = 0;
volatile uint8_t rxMode    = 0;  // extracted from control bits 1-4
volatile bool    i2cDataReceived = false;
volatile unsigned long lastI2CMillis = 0;

// --------------------- Misc ----------------------

bool debugVal = DEBUG;
bool lastBlanked = false;

// -------------------------------------------------

// ************************************************************
// Enable the HV generator, return true if we really turned it on
// ************************************************************
bool enableHV() {
  if (digitalRead(HVEnable) == LOW) {
    debugManager.debugMsg("HV ON");
    digitalWrite(HVEnable, HIGH);
    return true;
  } else {
    return false;
  }
}

// ************************************************************
// Disable the HV generator, return true if we really turned it off
// ************************************************************
bool disableHV() {
  if (digitalRead(HVEnable) == HIGH) {
    debugManager.debugMsg("HV OFF");
    digitalWrite(HVEnable, LOW);
    return true;
  } else {
    return false;
  }
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

  currentPos2 = phaseStep2 + digitStep2 * 3;
//  debugManager.debugMsg("G2 at " + String(currentPos2));

  G_step2(phaseStep2);
}

// ************************************************************
// Find the index mark
// ************************************************************
void findIndexMarks() {
  debugManager.debugMsg("findIndexMarks: searching...");
  int stepsDone = 0;
  bool found1, found2, found3, found4 = false;
  indexMark1 = -1;
  indexMark2 = -1;

  while (((indexMark1 < 0) | (indexMark2 < 0)) && (stepsDone < 100)) {
    if(indexMark1 < 0) {
      G1StepForwards();
      delay(10);
      if (digitalRead(Index1) == LOW) {
        indexMark1 = 0;
        found1 = true;
      }
    }

    if(indexMark2 < 0) {
      G2StepForwards();
      delay(10);
      if (digitalRead(Index2) == LOW) {
        indexMark2 = 0;
        found2 = true;
      }
    }

    stepsDone++;
  }

  indexMark1 = -1;
  indexMark2 = -1;
  stepsDone = 0;

  while (((indexMark1 < 0) | (indexMark2 < 0)) && (stepsDone < 100)) {
    if(indexMark1 < 0) {
      G1StepBackwards();
      delay(10);
      if (digitalRead(Index1) == LOW) {
        digitStep1 = 0;
        phaseStep1 = 0;
        indexMark1 = 0;
        found3 = true;
      }
    }

    if(indexMark2 < 0) {
      G2StepBackwards();
      delay(10);
      if (digitalRead(Index2) == LOW) {
        digitStep2 = 0;
        phaseStep2 = 0;
        indexMark2 = 0;
        found4 = true;
      }
    }

    stepsDone++;
  }

  G1StepForwards();
  delay(10);
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

  // We are at TDC and we want to be, so set the variables
  tdc1 = currentPos1;
  tdc2 = currentPos2;
  expPos1 = currentPos1;
  expPos2 = currentPos2;

  boolean foundAll = (found1 && found2) && (found3 && found4);

  if (!foundAll) {
    debugManager.debugMsg("findIndexMarks: WARNING - not all index marks found! found1=" + String(found1) + " found2=" + String(found2) + " found3=" + String(found3) + " found4=" + String(found4));
  } else {
    debugManager.debugMsg("findIndexMarks: all index marks found successfully");
    debugManager.debugMsg("findIndexMarks: done, tdc1=" + String(tdc1) + " tdc2=" + String(tdc2));
  }
}

// ************************************************************
// Move the expected position of Dec 1 back
// ************************************************************
void decExpPos1() {
  expPos1 = expPos1 + 1;
  if (expPos1 > 29) expPos1 = 0;
}

// ************************************************************
// Move the expected position of Dec 2 back
// ************************************************************
void decExpPos2() {
  expPos2 = expPos2 + 1;
  if (expPos2 > 29) expPos2 = 0;
}


// ************************************************************
// Move the current position of Dec 1 to the expected position
// ************************************************************
void align1toExpPos() {
  if (currentPos1 != expPos1) G1StepForwards();
}

// ************************************************************
// Move the current position of Dec 1 to the expected position
// ************************************************************
void align2toExpPos() {
  if (currentPos2 != expPos2) G2StepForwards();
}

// ************************************************************
// Move the current position of Dec 1 to the expected position
// ************************************************************
void align1toTDC() {
  expPos1 = tdc1;
}

// ************************************************************
// Move the current position of Dec 1 to the expected position
// ************************************************************
void align2toTDC() {
  expPos2 = tdc2;
}

// ************************************************************
// Called once per second
// ************************************************************
void performOncePerSecondProcessing() {
  debugManager.debugMsg(
    "time=" + String(rxHour) + ":" + String(rxMinute) + ":" + String(rxSecond) +
    " mode=" + String(rxMode) +
    " pos=" + String(currentPos1) + "/" + String(currentPos2) +
    " exp=" + String(expPos1) + "/" + String(expPos2) +
    " tdc=" + String(tdc1) + "/" + String(tdc2)
  );
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
// Read incoming serial data
// Protocol: 0xAA, Hours, Minutes, Seconds, Control
//   Control bit 0: blanked
//   Control bits 1-4: primary display mode
// ************************************************************
void readSerial() {
  static uint8_t buf[SERIAL_PACKET_SIZE];
  static int bufPos = 0;
  static bool inPacket = false;

  while (Serial.available()) {
    uint8_t b = Serial.read();

    if (!inPacket) {
      if (b == SERIAL_START_BYTE) {
        inPacket = true;
        bufPos = 0;
      }
    } else {
      buf[bufPos++] = b;
      if (bufPos == SERIAL_PACKET_SIZE) {
        rxHour    = buf[0];
        rxMinute  = buf[1];
        rxSecond  = buf[2];
        rxControl = buf[3];
        rxMode    = (rxControl >> DECATRON_CTRL_MODE_SHIFT) & 0x0F;
        i2cDataReceived = true;
        inPacket = false;
        bufPos = 0;
      }
    }
  }
}

// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------  Set up  --------------------------------------------
// ----------------------------------------------------------------------------------------------------
void setup() {
  pinMode(Guide1_1, OUTPUT);
  pinMode(Guide2_1, OUTPUT);
  pinMode(Index1, INPUT);

  pinMode(Guide1_2, OUTPUT);
  pinMode(Guide2_2, OUTPUT);
  pinMode(Index2, INPUT);

  pinMode(HVEnable, OUTPUT);

  Serial.begin(115200);
  debugManager.setUp(debugVal);

  debugManager.debugMsg("Started");

  nowMillis = millis();
  lastSecMillis = nowMillis;

  enableHV();

  debugManager.debugMsg("Find Index Mark");
  findIndexMarks();

  // Show us the index marks!!
  delay(1000);

  debugManager.debugMsg("Start Serial");

  lastI2CMillis = millis(); // prevent immediate blanking on startup

  debugManager.debugMsg("Startup done");
}

// ----------------------------------------------------------------------------------------------------
// -----------------------------------------------  Loop  ---------------------------------------------
// ----------------------------------------------------------------------------------------------------
void loop() {
  nowMillis = millis();

  // ------------------- Read incoming serial data -----------------------
  readSerial();

  // ------------------- Once-per-second debug logging -------------------

  if (nowMillis - lastSecMillis >= 1000) {
    lastSecMillis = nowMillis;
    performOncePerSecondProcessing();
  }

  // ------------------- Blanking ----------------------------------------
  // Blank if the master says to, or if we haven't heard from it in 5s

  blanked = (rxControl & DECATRON_CTRL_BLANKED) || ((nowMillis - lastI2CMillis) > 5000);

  if (blanked != lastBlanked) {
    debugManager.debugMsg(blanked ? "Blanked" : "Unblanked");
    lastBlanked = blanked;
  }

  if (blanked) {
    disableHV();
  } else {
    if (enableHV()) {
      // HV was just re-enabled — re-home the decatrons
      findIndexMarks();
      align1toTDC();
      align2toTDC();
    }
  }

  // ------------------- Handle received serial data ---------------------
  // Mode 0: Dec1 = minutes/2 (0-29), Dec2 = seconds/2 (0-29)

  if (i2cDataReceived) {
    lastI2CMillis = nowMillis;
    expPos1 = (tdc1 + rxMinute / 2) % 30;
    expPos2 = (tdc2 + rxSecond / 2) % 30;
    debugManager.debugMsg("RX: " + String(rxHour) + ":" + String(rxMinute) + ":" + String(rxSecond) +
      " mode=" + String(rxMode) + " blanked=" + String(rxControl & DECATRON_CTRL_BLANKED) +
      " -> expPos1=" + String(expPos1) + " expPos2=" + String(expPos2));
    i2cDataReceived = false;
  }

  align1toExpPos();
  align2toExpPos();

  // This is the speed of the animation
  delay(3);
}
