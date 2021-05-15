#pragma once

#include <Arduino.h>

// count0 is used to flah the status led 
#define COUNT0_MAX 1000
#define COUNT0_OFF 100

void startTimers();
void setLedFlashType(byte flashType);