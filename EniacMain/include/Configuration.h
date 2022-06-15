#pragma once

// -------------------------------------------------------------------------------
// This files hold high level hardware configurations
// -------------------------------------------------------------------------------

#define PROTO4                      // Proto 2 and below has the encoder B and btn3 swapped 

// -------------------------------------------------------------------------------

// Add debug statments to code - needs extra space
#define DEBUG_ON                    // DEBUG_ON | DEBUG_OFF

// -------------------------------------------------------------------------------

// Add the dignostic calls to the GUI
#define DIGIT_DIAGNOSTICS           // DIGIT_DIAGNOSTICS | DIGIT_DIAGNOSTICS_OFF

// -------------------------------------------------------------------------------

// Define the type of OLED
#define OLED_SH1106                 // OLED_SH1106 |  OLED_SSD1306

#define OLED_ON_TIME  20            // Time in seconds the OLED stays on for
#define CONFIG_TIME   10            // Time in seconds we stay in config mode
#define FLASH_TIME     6            // Time in seconds we show an OLED flash message for

// -------------------------------------------------------------------------------

// include the NeoPixels in the separator towers
#define FEATURE_SEP_LED_OFF         // FEATURE_SEP_LED | FEATURE_SEP_LED_OFF

// Define the type of NeoPixels
#define WS2812B                     // APA106, WS2812B

// If we output the NeoPixel string reversed
#define NORMAL_BL_OUTPUT            // REVERSE_BL_OUTPUT | NORMAL_BL_OUTPUT

// If we output the underlight string reversed
#define NORMAL_UL_OUTPUT            // REVERSE_UL_OUTPUT | NORMAL_UL_OUTPUT

// If we want a slave or not
#define SLAVE_OUTPUT_OFF            // SLAVE_OUTPUT | SLAVE_OUTPUT_OFF

// -------------------------------------------------------------------------------

#define SOFTWARE_VERSION "LTC-ESP32 V0.4"
#define CLOCK_MENU_TITLE "ENIAC" 

// -------------------------------------------------------------------------------

// Paul's special thing
#define COG_CRANK_OUTPUT_OFF        // COG_CRANK_OUTPUT | COG_CRANK_OUTPUT_OFF