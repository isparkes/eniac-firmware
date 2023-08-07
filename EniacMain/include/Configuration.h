#pragma once

// Uses PlatformIO package ESP32 v5.2.0

// -------------------------------------------------------------------------------
// This files hold high level hardware configurations
// -------------------------------------------------------------------------------

// Add debug statments to code - needs extra space
#define DEBUG_ON                    // DEBUG_ON | DEBUG_OFF

// -------------------------------------------------------------------------------

// Add the dignostic calls to the GUI
#define DIGIT_DIAGNOSTICS           // DIGIT_DIAGNOSTICS | DIGIT_DIAGNOSTICS_OFF

// -------------------------------------------------------------------------------

// invert the sense of the front panel switches
#define NORMAL_SWITCHES             // INVERT_SWITCHES | NORMAL_SWITCHES

// -------------------------------------------------------------------------------

// Countdown functionality
#define COUNTDOWN_OFF               // COUNTDOWN | COUNTDOWN_OFF

// -------------------------------------------------------------------------------

// Define the type of OLED
#define OLED_SSD1306                // OLED_SH1106 (1.3") |  OLED_SSD1306 (0.96" and 2.4")

#define OLED_ON_TIME  20            // Time in seconds the OLED stays on for
#define CONFIG_TIME   10            // Time in seconds we stay in config mode
#define FLASH_TIME     6            // Time in seconds we show an OLED flash message for

// -------------------------------------------------------------------------------

// If we want support for NeoPixel backlights 
#define FEATURE_BACKLIGHTS          // FEATURE_BACKLIGHTS | FEATURE_BACKLIGHTS_OFF

// include the NeoPixels in the separator towers
#define FEATURE_SEP_LED             // FEATURE_SEP_LED | FEATURE_SEP_LED_OFF

// Define the type of NeoPixels
#define WS2812B                     // APA106, WS2812B

// If we output the NeoPixel string reversed
#define NORMAL_BL_OUTPUT            // REVERSE_BL_OUTPUT | NORMAL_BL_OUTPUT

// If we output the underlight string reversed
#define NORMAL_UL_OUTPUT            // REVERSE_UL_OUTPUT | NORMAL_UL_OUTPUT

// If we output the digits reversed
#define REVERSE_DIGIT_OUTPUT         // REVERSE_DIGIT_OUTPUT | NORMAL_DIGIT_OUTPUT

// -------------------------------------------------------------------------------

// If we want support for neon blinkenlights
#define FEATURE_BLINKENLIGHTS       // FEATURE_BLINKENLIGHTS | FEATURE_BLINKENLIGHTS_OFF

// -------------------------------------------------------------------------------

// What type of slave we have: Either Nixie slave over I2C or the Decatron slave over the 1PPS GPIO
// If you don't want a slave, set "DECATRON_SLAVE"
#define DECATRON_SLAVE              // DECATRON_SLAVE | NIXIE_SLAVE

// -------------------------------------------------------------------------------

#define SOFTWARE_VERSION "LTC-ESP32 0.5.5.7"
#define CLOCK_MENU_TITLE "ENIAC" 

// -------------------------------------------------------------------------------

// Paul's special thing
#define COG_CRANK_OUTPUT_OFF        // COG_CRANK_OUTPUT | COG_CRANK_OUTPUT_OFF

// -------------------------------------------------------------------------------

