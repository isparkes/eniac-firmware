#include "clock_timers.h"
#include "defs.h"

hw_timer_t * timer0 = NULL;
portMUX_TYPE timerMux0 = portMUX_INITIALIZER_UNLOCKED;

hw_timer_t * timer1 = NULL;
portMUX_TYPE timerMux1 = portMUX_INITIALIZER_UNLOCKED;

volatile int count0;
volatile int count0Max = COUNT0_MAX;
volatile int count0Off = COUNT0_OFF;

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

void IRAM_ATTR onTimer1() {
   portENTER_CRITICAL_ISR(&timerMux1);
   count1++;
   portEXIT_CRITICAL_ISR(&timerMux1);
}

void startTimers() {

  pinMode(LED_PIN, OUTPUT);

  setLedFlashType(1);

  timer0 = timerBegin(0, 80, true);
  timerAttachInterrupt(timer0, &onTimer0, true);
  timerAlarmWrite(timer0, 1000, true);
  timerAlarmEnable(timer0);

  timer1 = timerBegin(1, 80, true);
  timerAttachInterrupt(timer1, &onTimer1, true);
  timerAlarmWrite(timer1, 33333, true);
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

