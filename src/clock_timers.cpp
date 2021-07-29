#include "clock_timers.h"
#include "defs.h"

hw_timer_t * timer0 = NULL;
portMUX_TYPE timerMux0 = portMUX_INITIALIZER_UNLOCKED;

hw_timer_t * timer1 = NULL;
portMUX_TYPE timerMux1 = portMUX_INITIALIZER_UNLOCKED;

volatile int count0;
volatile int count0Max = COUNT0_MAX;
volatile int count0Off = COUNT0_OFF;

extern volatile uint32_t val1;
extern volatile uint32_t val2;
extern volatile uint32_t val3;

volatile uint32_t val1curr = 0;
volatile uint32_t val2curr = 0;
volatile uint32_t val3curr = 0;

// count1 is used to output the display
volatile int count1;

// Led Timer
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
    digitalWrite(data1, !!(_val1 & (1 << (23 - i))));
    digitalWrite(clk1, HIGH);
    digitalWrite(clk1, LOW);
  }
  digitalWrite(latch1, HIGH);
  digitalWrite(latch1, LOW);
}

// ************************************************************
// Perform the parallel shift out to the registers
// ************************************************************
void IRAM_ATTR shiftOut24M(uint32_t _val1) {
  uint8_t i;

  for (i = 0; i < 24; i++) {
    digitalWrite(data2, !!(_val1 & (1 << (23 - i))));
    digitalWrite(clk1, HIGH);
    digitalWrite(clk1, LOW);
  }
  digitalWrite(latch2, HIGH);
  digitalWrite(latch2, LOW);
}

// ************************************************************
// Perform the parallel shift out to the registers
// ************************************************************
void IRAM_ATTR shiftOut24S(uint32_t _val1) {
  uint8_t i;

  for (i = 0; i < 24; i++) {
    digitalWrite(data3, !!(_val1 & (1 << (23 - i))));
    digitalWrite(clk1, HIGH);
    digitalWrite(clk1, LOW);
  }
  digitalWrite(latch3, HIGH);
  digitalWrite(latch3, LOW);
}

void IRAM_ATTR onTimer1() {
   portENTER_CRITICAL_ISR(&timerMux1);
   count1++;
   if (val1 != val1curr) {
     shiftOut24H(val1);
     val1curr = val1;
   } 
   if (val2 != val2curr) {
     shiftOut24M(val2);
     val2curr = val2;
   }
   if (val3 != val3curr) {
     shiftOut24S(val3);
     val3curr = val3;
   }
   portEXIT_CRITICAL_ISR(&timerMux1);
}

void startTimers() {
  // ledcSetup(PWMChannel, PWMFreq, PWMResolution);
  // ledcAttachPin(BLANK_PIN, PWMChannel);
  // setLedFlashType(1);
  // ledcWrite(PWMChannel, MAX_DUTY_CYCLE/2);

  timer0 = timerBegin(0, 80, true);
  timerAttachInterrupt(timer0, &onTimer0, true);
  timerAlarmWrite(timer0, 1000, true);
  timerAlarmEnable(timer0);

  timer1 = timerBegin(1, 80, true);
  timerAttachInterrupt(timer1, &onTimer1, true);
  timerAlarmWrite(timer1, 50000, true);
  timerAlarmEnable(timer1);

  setLedFlashType(1);
}

void setLedFlashType(byte flashType) {
  switch(flashType) {
    case 0: {
      count0Max = 1000;
      count0Off = 1;
      break;
    }
    case 1: {
      count0Max = 1000;
      count0Off = 500;
      break;
    }
  }
}

