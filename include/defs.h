#pragma once

#include <Arduino.h>

#define DIGIT_COUNT 6

#define INTERVAL_WIFI 10000
#define INTERVAL_WPS 10000
#define INTERVAL_PORTAL 300000

#define WDT_TIMEOUT 5

#define DEBUG_ON             // DEBUG_ON | DEBUG_OFF 

#define LED_PIN 2

#define SOFTWARE_VERSION "EPS32 NBZ V0.1"

#define COUNTS_PER_DIGIT 20

#define CLKPin    17
#define BLANKPin  18

#define DATA1Pin  23
#define LATCH1Pin 25

#define DATA2Pin  26
#define LATCH2Pin 27

#define DATA3Pin  32
#define LATCH3Pin 33

#define encoderA  34
#define encoderB  35

#define SDAint    21
#define SCLint    22

#define PIRPin    19

#define btn1      15
#define btn2      4

#define LDRPin    34

// -------------------------------------------------------------------------------

#define HOUR_MODE_DEFAULT               false
#define LEAD_BLANK_DEFAULT              false
#define SCROLLBACK_DEFAULT              false
#define FADE_DEFAULT                    false

// -------------------------------------------------------------------------------
// How quickly the scroll works
#define SCROLL_STEPS_DEFAULT 8
#define SCROLL_STEPS_MIN     1
#define SCROLL_STEPS_MAX     80

// -------------------------------------------------------------------------------
// The number of dispay impessions we need to fade by default
// 100 is about 1 second
#define FADE_STEPS_DEFAULT 50
#define FADE_STEPS_MIN     20
#define FADE_STEPS_MAX     200

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
#define DAY_BLANKING_MIN                0
#define DAY_BLANKING_NEVER              0  // Don't blank ever (default)
#define DAY_BLANKING_WEEKEND            1  // Blank during the weekend
#define DAY_BLANKING_WEEKDAY            2  // Blank during weekdays
#define DAY_BLANKING_ALWAYS             3  // Always blank
#define DAY_BLANKING_HOURS              4  // Blank between start and end hour every day
#define DAY_BLANKING_WEEKEND_OR_HOURS   5  // Blank between start and end hour during the week AND all day on the weekend
#define DAY_BLANKING_WEEKDAY_OR_HOURS   6  // Blank between start and end hour during the weekends AND all day on week days
#define DAY_BLANKING_WEEKEND_AND_HOURS  7  // Blank between start and end hour during the weekend
#define DAY_BLANKING_WEEKDAY_AND_HOURS  8  // Blank between start and end hour during week days
#define DAY_BLANKING_MAX                8
#define DAY_BLANKING_DEFAULT            0

// -------------------------------------------------------------------------------
#define BLANK_MODE_MIN                  0
#define BLANK_MODE_TUBES                0  // Use blanking for tubes only 
#define BLANK_MODE_LEDS                 1  // Use blanking for LEDs only
#define BLANK_MODE_BOTH                 2  // Use blanking for tubes and LEDs
#define BLANK_MODE_MAX                  2
#define BLANK_MODE_DEFAULT              2

// -------------------------------------------------------------------------------
#define ANTI_GHOST_MIN                  0
#define ANTI_GHOST_MAX                  50
#define ANTI_GHOST_DEFAULT              0

// -------------------------------------------------------------------------------
#define PIR_TIMEOUT_MIN                 60    // 1 minute in seconds
#define PIR_TIMEOUT_MAX                 3600  // 1 hour in seconds
#define PIR_TIMEOUT_DEFAULT             300   // 5 minutes in seconds

// -------------------------------------------------------------------------------
#define USE_LDR_DEFAULT                 true

// -------------------------------------------------------------------------------
#define SLOTS_MODE_MIN                  0
#define SLOTS_MODE_NONE                 0   // Don't use slots effect
#define SLOTS_MODE_1M_SCR_SCR           1   // use slots effect every minute, scroll in, scramble out
#define SLOTS_MODE_MAX                  1
#define SLOTS_MODE_DEFAULT              1

// -------------------------------------------------------------------------------
#define SEPARATOR_DIM_FACTOR_MIN        10
#define SEPARATOR_DIM_FACTOR_MAX        100
#define SEPARATOR_DIM_FACTOR_DEFAULT    100

// -------------------------------------------------------------------------------
#define BACKLIGHT_DIM_FACTOR_MIN        10
#define BACKLIGHT_DIM_FACTOR_MAX        100
#define BACKLIGHT_DIM_FACTOR_DEFAULT    100

#define USE_PIR_PULLUP_DEFAULT          true

// -------------------------------------------------------------------------------
// -------------------------------------------------------------------------------
// -------------------------------------------------------------------------------
// --------------------------- Strategy Backlights -------------------------------
#define BACKLIGHT_MIN                   0
#define BACKLIGHT_FIXED                 0  // Just define a colour and stick to it
#define BACKLIGHT_CYCLE                 1  // cycle through random colours, strategy 3
#define BACKLIGHT_COLOUR_TIME           2  // use "ColourTime" - different colours for each digit value
#define BACKLIGHT_DAY_OF_WEEK           3  // use "DayOfWeek" - different colours for each day
#define BACKLIGHT_MAX                   3
#define BACKLIGHT_DEFAULT               1

// -------------------------------------------------------------------------------
#define CYCLE_SPEED_MIN                 4
#define CYCLE_SPEED_MAX                 64
#define CYCLE_SPEED_DEFAULT             10

// -------------------------------------------------------------------------------
#define COLOUR_CNL_MAX                  15
#define COLOUR_RED_CNL_DEFAULT          15
#define COLOUR_GRN_CNL_DEFAULT          0
#define COLOUR_BLU_CNL_DEFAULT          0
#define COLOUR_CNL_MIN                  0

// -------------------------------------------------------------------------------
#define STATUS_RED                      0
#define STATUS_YELLOW                   1
#define STATUS_GREEN                    2
#define STATUS_BLUE                     3

// -------------------------------------------------------------------------------
#define LED_MODE_MIN        0
#define LED_RAILROAD        0
#define LED_BLINK_SLOW      1
#define LED_BLINK_FAST      2
#define LED_BLINK_DBL       3
#define LED_ON              4
#define LED_OFF             5
#define LED_BLINK_DEFAULT   LED_RAILROAD
#define LED_RAILROAD_X      -1 // Not yet implemented
#define LED_MODE_MAX        5

// -------------------------------------------------------------------------------
#define BACKLIGHT_DIM_FACTOR_MIN        10
#define BACKLIGHT_DIM_FACTOR_MAX        100
#define BACKLIGHT_DIM_FACTOR_DEFAULT    100

// -------------------------------------------------------------------------------
#define EXT_DIM_FACTOR_MIN              10
#define EXT_DIM_FACTOR_MAX              100
#define EXT_DIM_FACTOR_DEFAULT          100
