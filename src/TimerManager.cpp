#include "TimerManager.h"
#include "defs.h"

hw_timer_t * timer0 = NULL;
portMUX_TYPE timerMux0 = portMUX_INITIALIZER_UNLOCKED;

hw_timer_t * timer1 = NULL;
extern portMUX_TYPE timerMux1;

hw_timer_t * timer2 = NULL;
portMUX_TYPE timerMux2 = portMUX_INITIALIZER_UNLOCKED;

volatile int count0;
volatile int count0Max = COUNT0_MAX;
volatile int count0Off = COUNT0_OFF;

// These variables hold the impression data
extern volatile uint32_t val1;
extern volatile uint32_t val2;
extern volatile uint32_t val3;

extern volatile uint32_t nextVal1;
extern volatile uint32_t nextVal2;
extern volatile uint32_t nextVal3;

extern volatile uint8_t switchTime;

// These are for debugging
extern volatile uint16_t impressions;

// These strange values are to provoke that the first call to
// the interrups detects a change in the buffer and outputs
// the 0 values to the display. This avoids ghosting of digits
// during set up of the display  
volatile uint8_t _phase;

volatile uint8_t _switchTime;

volatile uint32_t _val1curr = 1;
volatile uint32_t _val2curr = 1;
volatile uint32_t _val3curr = 1;

volatile uint32_t _val1 = 0;
volatile uint32_t _val2 = 0;
volatile uint32_t _val3 = 0;

volatile uint32_t _val1Next = 0;
volatile uint32_t _val2Next = 0;
volatile uint32_t _val3Next = 0;

// ************************************************************
// ISR for LED flash update
// ************************************************************
void IRAM_ATTR onTimer0() {
   portENTER_CRITICAL_ISR(&timerMux0);
   count0++;
   if (count0 > count0Max) {
     count0 = 0;
     digitalWrite(LED_PIN, HIGH);
   } else if (count0 == count0Off) {
     digitalWrite(LED_PIN, LOW);
   }
   portEXIT_CRITICAL_ISR(&timerMux0);
}

// ************************************************************
// Perform the parallel shift out to the registers
// ************************************************************
void IRAM_ATTR shiftOut24H(uint32_t _val1) {
  uint8_t i;

  for (i = 0; i < 24; i++) {
    digitalWrite(DATA1Pin, !!(_val1 & (1 << (23 - i))));
    digitalWrite(CLKPin, HIGH);
    digitalWrite(CLKPin, HIGH);
    digitalWrite(CLKPin, LOW);
    digitalWrite(CLKPin, LOW);
  }
  digitalWrite(LATCH1Pin, HIGH);
  digitalWrite(LATCH1Pin, HIGH);
  digitalWrite(LATCH1Pin, LOW);
  digitalWrite(LATCH1Pin, LOW);
}

// ************************************************************
// Perform the parallel shift out to the registers
// ************************************************************
void IRAM_ATTR shiftOut24M(uint32_t _val1) {
  uint8_t i;

  for (i = 0; i < 24; i++) {
    digitalWrite(DATA2Pin, !!(_val1 & (1 << (23 - i))));
    digitalWrite(CLKPin, HIGH);
    digitalWrite(CLKPin, HIGH);
    digitalWrite(CLKPin, LOW);
    digitalWrite(CLKPin, LOW);
  }
  digitalWrite(LATCH2Pin, HIGH);
  digitalWrite(LATCH2Pin, HIGH);
  digitalWrite(LATCH2Pin, LOW);
  digitalWrite(LATCH2Pin, LOW);
}

// ************************************************************
// Perform the parallel shift out to the registers
// ************************************************************
void IRAM_ATTR shiftOut24S(uint32_t _val1) {
  uint8_t i;

  for (i = 0; i < 24; i++) {
    digitalWrite(DATA3Pin, !!(_val1 & (1 << (23 - i))));
    digitalWrite(CLKPin, HIGH);
    digitalWrite(CLKPin, HIGH);
    digitalWrite(CLKPin, LOW);
    digitalWrite(CLKPin, LOW);
  }
  digitalWrite(LATCH3Pin, HIGH);
  digitalWrite(LATCH3Pin, HIGH);
  digitalWrite(LATCH3Pin, LOW);
  digitalWrite(LATCH3Pin, LOW);
}

// ************************************************************
// ISR for display update
// ************************************************************
void IRAM_ATTR onTimer1() {
  portENTER_CRITICAL_ISR(&timerMux1);
  _phase++;
    if (_phase >= PHASE_MAX) {
    _phase = 0;
    impressions++;

    // Load the new values
    _switchTime = switchTime;
    _val1 = val1;
    _val2 = val2;
    _val3 = val3;
    _val1Next = nextVal1;
    _val2Next = nextVal2;
    _val3Next = nextVal3;

    if (_val1 != _val1curr) {
      shiftOut24H(_val1);
      _val1curr = _val1;
    } 
    if (_val2 != _val2curr) {
      shiftOut24M(_val2);
      _val2curr = _val2;
    }
    if (_val3 != _val3curr) {
      shiftOut24S(_val3);
      _val3curr = _val3;
    }
  }

  if (_phase == _switchTime) {
    if (_val1Next != _val1curr) {
      shiftOut24H(_val1Next);
      _val1curr = _val1Next;
    } 
    if (_val2Next != _val2curr) {
      shiftOut24M(_val2Next);
      _val2curr = _val2Next;
    }
    if (_val3Next != _val3curr) {
      shiftOut24S(_val3Next);
      _val3curr = _val3Next;
    }
  }
  portEXIT_CRITICAL_ISR(&timerMux1);
}

// ************************************************************
// ISR for 1PPS output
// ************************************************************
void IRAM_ATTR onTimer2() {
  portENTER_CRITICAL_ISR(&timerMux2);
  digitalWrite(PPSPin, LOW);
  portEXIT_CRITICAL_ISR(&timerMux2);
}

// ************************************************************
// Trigger 1PPS output
// ************************************************************
void triggerOnePulsePerSec() {
  digitalWrite(PPSPin, HIGH);
  timerRestart(timer2);
  timerAlarmEnable(timer2);
}

// ************************************************************
// Start the timers
// ************************************************************
void startTimers() {
  // LED flash timer
  timer0 = timerBegin(0, 80, true);
  timerAttachInterrupt(timer0, &onTimer0, true);
  timerAlarmWrite(timer0, 10000, true);
  // https://community.platformio.org/t/hardware-timer-issue-with-esp32/22047/10
  delayMicroseconds(0);
  timerAlarmEnable(timer0);

  // Display time
  timer1 = timerBegin(1, 80, true);
  timerAttachInterrupt(timer1, &onTimer1, true);
  timerAlarmWrite(timer1, 500, true);
  // https://community.platformio.org/t/hardware-timer-issue-with-esp32/22047/10
  delayMicroseconds(0);
  timerAlarmEnable(timer1);

  // 1PPS timer
  timer2 = timerBegin(2, 80, true);
  timerAttachInterrupt(timer2, &onTimer2, true);
  // https://community.platformio.org/t/hardware-timer-issue-with-esp32/22047/10
  timerAlarmWrite(timer2, 50000, false);
  delayMicroseconds(0);
  timerAlarmEnable(timer2);

  // Set default LED flash type
  setLedFlashType(1);
}

// ************************************************************
// Set the LED flash type
// 0: Connected - short flash 1/s
// 1: Connecting - long flash 2/3s
// ************************************************************
void setLedFlashType(byte flashType) {
  switch(flashType) {
    case 0: {
      count0Max = 100;
      count0Off = 1;
      break;
    }
    case 1: {
      count0Max = 100;
      count0Off = 50;
      break;
    }
  }
}

