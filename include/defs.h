#pragma once

#include <Arduino.h>

#define INTERVAL_WIFI 10000
#define INTERVAL_WPS 10000
#define INTERVAL_PORTAL 300000

#define WDT_TIMEOUT 3

#define DEBUG_ON             // DEBUG_ON | DEBUG_OFF 

#define LED_PIN 2

#define SOFTWARE_VERSION "EPS32 NBZ V0.1"

// #define CLK_PIN 17
// #define BLANK_PIN 18

// #define DATA1_PIN 23
// #define LATCH1_PIN 25

// #define DATA2_PIN 26
// #define LATCH2_PIN 27

// #define DATA3_PIN 32
// #define LATCH3_PIN 33

const uint8_t PixelCount = 14;
const uint8_t PixelPin = 13;

const uint8_t clk1 = 17;
const uint8_t blnk1 = 18;

const uint8_t data1 = 23;
const uint8_t latch1 = 25;

const uint8_t data2 = 26;
const uint8_t latch2 = 27;

const uint8_t data3 = 32;
const uint8_t latch3 = 33;

const uint8_t encoderA = 34;
const uint8_t encoderB = 35;

const uint8_t SDAinternal = 21;
const uint8_t SCLinternal = 22;

const uint8_t PIR = 19;

const uint8_t btn1 = 15;

const uint8_t DLS = 4;
