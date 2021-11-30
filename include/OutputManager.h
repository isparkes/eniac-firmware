#pragma once

#include "Arduino.h"
#include "globals.h"
#include "utilities.h"
#include "defs.h"
#include "BlankingManager.h"

#define APPLY_LEAD_0_BLANK true
#define DO_NOT_APPLY_LEAD_0_BLANK false

void loadNumberArrayTime();
void loadNumberArrayDate();
void loadNumberArraySameValue(byte value);

void allNormal(bool leadingBlank);

void outputDisplay();
void allBlanked();

void applyBlanking();

uint32_t decodeFromNumberArray(byte valueToDecodeTens, byte valueToDecodeUnits, bool blankTens, bool blankUnits, bool bl1, bool bl2, bool led1, bool led2);
