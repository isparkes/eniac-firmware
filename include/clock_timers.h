#pragma once

#include <Arduino.h>

// count0 is used to flah the status led 
#define COUNT0_MAX 1000
#define COUNT0_OFF 100

// ************************************************************
// Public prototypes
// ************************************************************
void startTimers();
void setLedFlashType(byte flashType);
int getCount0();
void triggerTimer2();

void IRAM_ATTR shiftOut24H(uint32_t _val1);
void IRAM_ATTR shiftOut24M(uint32_t _val2);
void IRAM_ATTR shiftOut24S(uint32_t _val3);