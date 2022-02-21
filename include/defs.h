#pragma once

#include <Arduino.h>

// -------------------------------------------------------------------------------

#define PROTO4               // Proto 2 and below has the encoder B and btn3 swapped 

// -------------------------------------------------------------------------------

#define DIGIT_COUNT 6

#define WDT_TIMEOUT 5

// Add debug statments to code - needs extra space
#define DEBUG_ON           // DEBUG_ON | DEBUG_OFF

// Add the dignostic calls to the GUI
#define DIGIT_DIAGNOSTICS    // DIGIT_DIAGNOSTICS | DIGIT_DIAGNOSTICS_OFF

#define SOFTWARE_VERSION "LTC-ESP32 V0.2"

#define COUNTS_PER_DIGIT 20

#define SERIAL_BAUD_RATE 115200

#define SLAVE_MODULE_I2C_ADDRESS 105

// Onboard LED 
#define LED_PIN 2

// Digit drivers
#define CLKPin    19
#define BLANKPin  18

#define DATA1Pin  23
#define LATCH1Pin 17

#define DATA2Pin  26
#define LATCH2Pin 27

#define DATA3Pin  32
#define LATCH3Pin 33

// Encoder
#define ENC_APin  5
#ifdef PROTO2
    #define ENC_BPin  12
#else
    #define ENC_BPin  14
#endif
#define ENC_BTN   16

// Internally defined
#define SDAint    21
#define SCLint    22

// Touch capable buttons
#define BTN1Pin   15
#define BTN2Pin   4
#ifdef PROTO2
    #define BTN3Pin   14
#else
    #define BTN3Pin   12
#endif

// Analogue capable - no internal pullups
#define LDRPin    34
#define PIRPin    35

#define PPSPin    0

#define LED_DOUT  13

// -------------------------------------------------------------------------------

#define DIGIT_COUNT 6

#define PHASE_MAX 20
#define H10 0
#define H1  1
#define M10 2
#define M1  3
#define S10 4
#define S1  5

// -------------------------------------------------------------------------------

#define TOUCH_THRESHOLD 60

// -------------------------------------------------------------------------------

#define HOUR_MODE_DEFAULT               false
#define LEAD_BLANK_DEFAULT              false
#define SCROLLBACK_DEFAULT              false
#define FADE_DEFAULT                    false

// -------------------------------------------------------------------------------
// Temporary display modes - accessed by a short press ( < 1S ) on the button when in MODE_TIME
#define TEMP_MODE_MIN                   0
#define TEMP_MODE_DATE                  0 // Display the date for 5 S
#define TEMP_MODE_LDR                   1 // Display the normalised LDR reading for 5S, returns a value from 100 (dark) to 999 (bright)
#define TEMP_MODE_VERSION               2 // Display the version for 5S
#define TEMP_IP_ADDR12                  3 // IP xxx.yyy.zzz.aaa: xxx.yyy
#define TEMP_IP_ADDR34                  4 // IP xxx.yyy.zzz.aaa: zzz.aaa
#define TEMP_IMPR                       5 // number of impressions per second
#define TEMP_MODE_MAX                   5

#define TEMP_DISPLAY_MODE_DUR_MS        5000

// -------------------------------------------------------------------------------
#define DATE_FORMAT_MIN                 0
#define DATE_FORMAT_YYMMDD              0
#define DATE_FORMAT_MMDDYY              1
#define DATE_FORMAT_DDMMYY              2
#define DATE_FORMAT_MAX                 2
#define DATE_FORMAT_DEFAULT             2

// -------------------------------------------------------------------------------

#define OLED_ON_TIME                    20 // Time in seconds the OLED stays on for
#define CONFIG_TIME                     10 // Time in seconds we stay in config mode
#define FLASH_TIME                       6 // Time in seconds we show an OLED flash message for

// -------------------------------------------------------------------------------
#define SUPPRESS_ACP_DEFAULT            false

// -------------------------------------------------------------------------------
#define DIGIT_DIAGS_MODE_NONE           0
#define DIGIT_DIAGS_MODE_FAST           1
#define DIGIT_DIAGS_MODE_SLOW           2
#define DIGIT_DIAGS_MODE_ENCODER        3

// -------------------------------------------------------------------------------
# define TIME_SOURCE_GPS                0
# define TIME_SOURCE_NTP                1
# define TIME_SOURCE_RTC                2
# define TIME_SOURCE_INT                3

// -------------------------------------------------------------------------------
#define SLAVE_MODE_MIN                  0
#define SLAVE_MODE_100THS               0
#define SLAVE_MODE_DATE                 1
#define SLAVE_MODE_MAX                  1
#define SLAVE_MODE_DEFAULT              0

