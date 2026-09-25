// ************************************************************
// Timer and interrupt handling
//
// Design notes for the display ISR (onTimer1) and LED ISR
// (onTimer0) - both are written to be as short as possible:
//
// - No digitalWrite(). It lives in flash (it wraps
//   gpio_set_level()), so each call from the ISR goes through
//   the flash cache and does a pin lookup at run time. We write
//   the GPIO W1TS/W1TC set/clear registers directly, with pin
//   masks worked out at compile time from Defs.h. Pins can move
//   (including to GPIO 32+) without touching this code.
//
// - Parallel shift. The three register chains share CLKPin, so
//   all three data lines are clocked together: 24 clocks instead
//   of 3 x 24. Only the chains that changed are latched. The
//   loop is branch free, so the timing does not depend on the
//   data.
//
// - Short critical section. timerMux1 is only held while
//   copying the values written by OutputManager, not while
//   shifting, so other interrupts on this core are not held off.
//
// - ISR private state is static and not volatile, so the
//   compiler can keep it in registers.
//
// - No assembler. The compiler already produces a branch free,
//   zero overhead hardware loop running from IRAM. The time is
//   spent on the GPIO register writes themselves, so hand
//   written assembler would gain only a few cycles per bit.
//
// Timing: the clock and latch pulses are much shorter than with
// digitalWrite(). Each clock level is held for two register
// writes, as in the old code. If the drivers miss bits (garbled
// digits), check CLKPin with a scope and add another
// GPIO_SET(CLK_B0, CLK_B1) in shiftOut24x3() to stretch the
// clock pulse.
// ************************************************************

#include "TimerManager.h"
#include "soc/gpio_reg.h"

hw_timer_t * timer0 = NULL;

hw_timer_t * timer1 = NULL;
extern portMUX_TYPE timerMux1;

// Only touched by the ISR - no need for volatile
static int count0 = 0;
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

// ISR private state. Not volatile: only the display ISR reads or
// writes these, so the compiler is free to keep them in registers.
//
// These strange values are to provoke that the first call to
// the interrups detects a change in the buffer and outputs
// the 0 values to the display. This avoids ghosting of digits
// during set up of the display  
static uint8_t _phase;

static uint8_t _switchTime;

static uint32_t _val1curr = 1;
static uint32_t _val2curr = 1;
static uint32_t _val3curr = 1;

static uint32_t _val1Next = 0;
static uint32_t _val2Next = 0;
static uint32_t _val3Next = 0;

// ************************************************************
// Direct GPIO register access.
//
// digitalWrite() lives in flash (it wraps gpio_set_level()), so
// every call from the ISR goes through the flash cache and does
// a pin lookup at run time. Instead we write the GPIO set/clear
// registers directly with masks worked out at compile time.
//
// GPIO 0-31 are in bank 0 (GPIO_OUT_W1TS/W1TC), GPIO 32-39 in
// bank 1 (GPIO_OUT1_W1TS/W1TC). The W1TS/W1TC registers only
// affect the bits we write, so this is safe against other code
// driving other pins.
// ************************************************************
#define BANK0(p) ((p) < 32 ? (1UL << (p)) : 0UL)
#define BANK1(p) ((p) < 32 ? 0UL : (1UL << ((p) - 32)))

static const uint32_t CLK_B0    = BANK0(CLKPin);
static const uint32_t CLK_B1    = BANK1(CLKPin);
static const uint32_t DATA1_B0  = BANK0(DATA1Pin);
static const uint32_t DATA1_B1  = BANK1(DATA1Pin);
static const uint32_t DATA2_B0  = BANK0(DATA2Pin);
static const uint32_t DATA2_B1  = BANK1(DATA2Pin);
static const uint32_t DATA3_B0  = BANK0(DATA3Pin);
static const uint32_t DATA3_B1  = BANK1(DATA3Pin);
static const uint32_t LATCH1_B0 = BANK0(LATCH1Pin);
static const uint32_t LATCH1_B1 = BANK1(LATCH1Pin);
static const uint32_t LATCH2_B0 = BANK0(LATCH2Pin);
static const uint32_t LATCH2_B1 = BANK1(LATCH2Pin);
static const uint32_t LATCH3_B0 = BANK0(LATCH3Pin);
static const uint32_t LATCH3_B1 = BANK1(LATCH3Pin);
static const uint32_t DATA_B0   = DATA1_B0 | DATA2_B0 | DATA3_B0;
static const uint32_t DATA_B1   = DATA1_B1 | DATA2_B1 | DATA3_B1;
static const uint32_t LATCH_B0  = LATCH1_B0 | LATCH2_B0 | LATCH3_B0;
static const uint32_t LATCH_B1  = LATCH1_B1 | LATCH2_B1 | LATCH3_B1;
static const uint32_t LED_B0    = BANK0(LED_PIN);
static const uint32_t LED_B1    = BANK1(LED_PIN);

// Set/clear bits b0 (bank 0) and b1 (bank 1). use0/use1 must be
// compile time constants: a bank with no pins in use is skipped
// without a run time test, so the write sequence (and therefore
// the timing) does not depend on the data.
#define GPIO_W1TS(use0, use1, b0, b1) do { if (use0) REG_WRITE(GPIO_OUT_W1TS_REG, (b0)); if (use1) REG_WRITE(GPIO_OUT1_W1TS_REG, (b1)); } while (0)
#define GPIO_W1TC(use0, use1, b0, b1) do { if (use0) REG_WRITE(GPIO_OUT_W1TC_REG, (b0)); if (use1) REG_WRITE(GPIO_OUT1_W1TC_REG, (b1)); } while (0)

// For constant masks
#define GPIO_SET(b0, b1) GPIO_W1TS(b0, b1, b0, b1)
#define GPIO_CLR(b0, b1) GPIO_W1TC(b0, b1, b0, b1)

// Branch free: all ones if bit 23 of _val is set, else all zeros, ANDed with the pin mask
#define BIT23_MASK(_val, _mask) ((uint32_t)(-(int32_t)(((_val) >> 23) & 1)) & (_mask))

// ************************************************************
// ISR for LED flash update
// ************************************************************
void IRAM_ATTR onTimer0() {
  if (++count0 > count0Max) {
    count0 = 0;
    GPIO_SET(LED_B0, LED_B1);
  } else if (count0 == count0Off) {
    GPIO_CLR(LED_B0, LED_B1);
  }
}

// ************************************************************
// Perform the parallel shift out to the registers.
//
// All three register chains share CLKPin, so we clock all three
// data lines at the same time: 24 clocks instead of 3 x 24. Only
// the chains whose latch bit is in latchB0/latchB1 get their
// outputs updated. The others just see new data in their shift
// stages, which is not visible until latched (and which happened
// before anyway, since the clock was always shared).
//
// Timing per bit: the data lines change at the same time as the
// clock falls, which is well after the previous rising edge
// (hold time). Each clock level is then held for two register
// writes, the same as the old code.
// ************************************************************
static inline void IRAM_ATTR shiftOut24x3(uint32_t v1, uint32_t v2, uint32_t v3, uint32_t latchB0, uint32_t latchB1) {
  for (uint8_t i = 0; i < 24; i++) {
    uint32_t setB0 = BIT23_MASK(v1, DATA1_B0) | BIT23_MASK(v2, DATA2_B0) | BIT23_MASK(v3, DATA3_B0);
    uint32_t setB1 = BIT23_MASK(v1, DATA1_B1) | BIT23_MASK(v2, DATA2_B1) | BIT23_MASK(v3, DATA3_B1);
    v1 <<= 1;
    v2 <<= 1;
    v3 <<= 1;

    // clock low, clear the data lines that should be 0, then set the ones that should be 1
    GPIO_W1TC(CLK_B0 | DATA_B0, CLK_B1 | DATA_B1, CLK_B0 | (DATA_B0 & ~setB0), CLK_B1 | (DATA_B1 & ~setB1));
    GPIO_W1TS(DATA_B0, DATA_B1, setB0, setB1);

    // clock high - data is shifted in on the rising edge
    GPIO_SET(CLK_B0, CLK_B1);
    GPIO_SET(CLK_B0, CLK_B1);
  }
  GPIO_CLR(CLK_B0, CLK_B1);

  // Latch the chains that changed
  GPIO_W1TS(LATCH_B0, LATCH_B1, latchB0, latchB1);
  GPIO_W1TS(LATCH_B0, LATCH_B1, latchB0, latchB1);
  GPIO_W1TC(LATCH_B0, LATCH_B1, latchB0, latchB1);
  GPIO_W1TC(LATCH_B0, LATCH_B1, latchB0, latchB1);
}

// ************************************************************
// Single chain versions, published so that we can access these
// during startup
// ************************************************************
void IRAM_ATTR shiftOut24H(uint32_t _val1) {
  shiftOut24x3(_val1, 0, 0, LATCH1_B0, LATCH1_B1);
}

void IRAM_ATTR shiftOut24M(uint32_t _val2) {
  shiftOut24x3(0, _val2, 0, LATCH2_B0, LATCH2_B1);
}

void IRAM_ATTR shiftOut24S(uint32_t _val3) {
  shiftOut24x3(0, 0, _val3, LATCH3_B0, LATCH3_B1);
}

// ************************************************************
// Output the values for any of the chains that have changed, in
// one parallel shift
// ************************************************************
static inline void IRAM_ATTR updateDisplay(uint32_t v1, uint32_t v2, uint32_t v3) {
  uint32_t latchB0 = 0;
  uint32_t latchB1 = 0;

  if (v1 != _val1curr) {
    latchB0 |= LATCH1_B0;
    latchB1 |= LATCH1_B1;
    _val1curr = v1;
  }
  if (v2 != _val2curr) {
    latchB0 |= LATCH2_B0;
    latchB1 |= LATCH2_B1;
    _val2curr = v2;
  }
  if (v3 != _val3curr) {
    latchB0 |= LATCH3_B0;
    latchB1 |= LATCH3_B1;
    _val3curr = v3;
  }

  if (latchB0 | latchB1) {
    shiftOut24x3(v1, v2, v3, latchB0, latchB1);
  }
}

// ************************************************************
// ISR for display update.
// Each display impression is made up of 20 phase steps. When
// cross-fading from one digit to another, we switch the display
// one one of the 20 steps ("switch time"). The fade is done by
// pregressively changing the switchTime value.
//
// Additionally we only outout to the shift register (3 8 bit
// shift registers for each digit pair = 24 bits) if we detect
// the value has changed.
//
// We also have an interlock between writing the data into the
// registers (val1,2,3, nextVal1,2,3, switchTime) using the
// timerMux1 mutex. If we don't have this, the display will
// glitch due to the values changing while displaying).
// This is double-buffering: The values are calculated into
// temporary variables in OutputManager_::outputDisplay, then
// loaded into the local variables ONCE PER IMPRESSION.
//
// The mutex is only held while copying the values, not while
// shifting, so other interrupts on this core are not held off
// for the duration of the shift.
// ************************************************************
void IRAM_ATTR onTimer1() {
  if (++_phase >= PHASE_MAX) {
    uint32_t v1, v2, v3;

    _phase = 0;

    // Load the new values from the output of the outputManager
    portENTER_CRITICAL_ISR(&timerMux1);
    impressions++;
    _switchTime = switchTime;
    v1 = val1;
    v2 = val2;
    v3 = val3;
    _val1Next = nextVal1;
    _val2Next = nextVal2;
    _val3Next = nextVal3;
    portEXIT_CRITICAL_ISR(&timerMux1);

    updateDisplay(v1, v2, v3);
  }

  if (_phase == _switchTime) {
    updateDisplay(_val1Next, _val2Next, _val3Next);
  }
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

  // Set default LED flash type
  setLedFlashType(1);

  // Hook up the switches to the trigger handler
  attachInterrupt(Switch1Pin, switchISR, CHANGE);
  attachInterrupt(Switch2Pin, switchISR, CHANGE);
}

// ************************************************************
// Switch changed - mark that there is an event waiting
// ************************************************************
void IRAM_ATTR switchISR() {
    switchEventWaiting = true;
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

