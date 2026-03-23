#pragma once

// Uses PlatformIO package ESP32 v6.9.0

// -------------------------------------------------------------------------------
// This files hold high level hardware configurations
// -------------------------------------------------------------------------------


// Config sets
#define CONFIG_ENIAC               // CONFIG_ENIAC | CONFIG_MINIAC

// -------------------------------------------------------------------------------
#define SOFTWARE_VERSION "LTC-ESP32 0.6.0.8"

#ifdef CONFIG_ENIAC
    // Add debug statments to code - needs extra space
    #define DEBUG_OFF                   // DEBUG | DEBUG_OFF

    // -------------------------------------------------------------------------------

    // Add the dignostic calls to the GUI
    #define DIGIT_DIAGNOSTICS_OFF       // DIGIT_DIAGNOSTICS | DIGIT_DIAGNOSTICS_OFF

    // -------------------------------------------------------------------------------

    // invert the sense of the front panel switches
    #define NORMAL_SWITCHES             // INVERT_SWITCHES | NORMAL_SWITCHES

    // -------------------------------------------------------------------------------

    // Countdown functionality
    #define COUNTDOWN                   // COUNTDOWN | COUNTDOWN_OFF

    // -------------------------------------------------------------------------------

    // Define the type of OLED
    #define OLED_SSD1306                // OLED_SH1106 (1.3") |  OLED_SSD1306 (0.96" and 2.4")

    // -------------------------------------------------------------------------------

    // If we want support for NeoPixel backlights 
    #define FEATURE_BACKLIGHTS          // FEATURE_BACKLIGHTS | FEATURE_BACKLIGHTS_OFF

    // If we want support for additional NeoPixels on the end of the main chain
    #define FEATURE_EXT_LEDS_OFF        // FEATURE_EXT_LEDS | FEATURE_EXT_LEDS_OFF

    // The number of pixels per tube - if only 1, hue offsets will not be available
    #define PIXELS_PER_TUBE 2           // 1 or 2

    // include the NeoPixels in the separator towers
    #define FEATURE_SEP_LED             // FEATURE_SEP_LED | FEATURE_SEP_LED_OFF

    // Define the type of NeoPixels
    #define WS2812B                     // APA106, WS2812B

    // If we output the NeoPixel string reversed
    #define NORMAL_BL_OUTPUT            // REVERSE_BL_OUTPUT | NORMAL_BL_OUTPUT

    // If we output the underlight string reversed
    #define NORMAL_UL_OUTPUT            // REVERSE_UL_OUTPUT | NORMAL_UL_OUTPUT

    // If we output the digits reversed
    #define NORMAL_DIGIT_OUTPUT         // REVERSE_DIGIT_OUTPUT | NORMAL_DIGIT_OUTPUT

    // -------------------------------------------------------------------------------

    // If we want support for neon blinkenlights
    #define FEATURE_BLINKENLIGHTS       // FEATURE_BLINKENLIGHTS | FEATURE_BLINKENLIGHTS_OFF

    // -------------------------------------------------------------------------------

    // Encoder / OLED support
    #define FEATURE_MENU                // FEATURE_MENU | FEATURE_MENU_OFF

    // -------------------------------------------------------------------------------

    // What type of slave we have: Either Nixie slave over I2C or the Decatron slave over the 1PPS GPIO
    // If you don't want a slave, set "DECATRON_SLAVE"
    #define DECATRON_SLAVE              // DECATRON_SLAVE | NIXIE_SLAVE

    #define CLOCK_MENU_TITLE "ENIAC" 

    // -------------------------------------------------------------------------------

    // Paul's special thing
    #define COG_CRANK_OUTPUT_OFF        // COG_CRANK_OUTPUT | COG_CRANK_OUTPUT_OFF

    // -------------------------------------------------------------------------------

    // Access to Crypto/FX tickers
    #define FEATURE_TICKER             // FEATURE_TICKER | FEATURE_TICKER_OFF

    // -------------------------------------------------------------------------------
#endif

#ifdef CONFIG_MINIAC
    // Add debug statments to code - needs extra space
    #define DEBUG                       // DEBUG | DEBUG_OFF

    // -------------------------------------------------------------------------------

    // Add the dignostic calls to the GUI
    #define DIGIT_DIAGNOSTICS_OFF       // DIGIT_DIAGNOSTICS | DIGIT_DIAGNOSTICS_OFF

    // -------------------------------------------------------------------------------

    // invert the sense of the front panel switches
    #define NORMAL_SWITCHES             // INVERT_SWITCHES | NORMAL_SWITCHES

    // -------------------------------------------------------------------------------

    // Countdown functionality
    #define COUNTDOWN_OFF               // COUNTDOWN | COUNTDOWN_OFF

    // -------------------------------------------------------------------------------

    // Define the type of OLED
    #define OLED_SH1106                 // OLED_SH1106 (1.3") |  OLED_SSD1306 (0.96" and 2.4")

    // -------------------------------------------------------------------------------

    // If we want support for NeoPixel backlights 
    #define FEATURE_BACKLIGHTS          // FEATURE_BACKLIGHTS | FEATURE_BACKLIGHTS_OFF

    // If we want support for additional NeoPixels on the end of the main chain
    #define FEATURE_EXT_LEDS_OFF        // FEATURE_EXT_LEDS | FEATURE_EXT_LEDS_OFF

    // The number of pixels per tube - if only 1, hue offsets will not be available
    #define PIXELS_PER_TUBE 2           // 1 or 2

    // include the NeoPixels in the separator towers
    #define FEATURE_SEP_LED             // FEATURE_SEP_LED | FEATURE_SEP_LED_OFF

    // Define the type of NeoPixels
    #define WS2812B                     // APA106, WS2812B

    // If we output the NeoPixel string reversed
    #define REVERSE_BL_OUTPUT           // REVERSE_BL_OUTPUT | NORMAL_BL_OUTPUT

    // If we output the underlight string reversed
    #define NORMAL_UL_OUTPUT            // REVERSE_UL_OUTPUT | NORMAL_UL_OUTPUT

    // If we output the digits reversed
    #define REVERSE_DIGIT_OUTPUT        // REVERSE_DIGIT_OUTPUT | NORMAL_DIGIT_OUTPUT

    // -------------------------------------------------------------------------------

    // If we want support for neon blinkenlights
    #define FEATURE_BLINKENLIGHTS       // FEATURE_BLINKENLIGHTS | FEATURE_BLINKENLIGHTS_OFF

    // -------------------------------------------------------------------------------

    // Encoder / OLED support
    #define FEATURE_MENU                // FEATURE_MENU | FEATURE_MENU_OFF

    // -------------------------------------------------------------------------------

    // What type of slave we have: Either Nixie slave over I2C or the Decatron slave over the 1PPS GPIO
    // If you don't want a slave, set "DECATRON_SLAVE"
    #define DECATRON_SLAVE              // DECATRON_SLAVE | NIXIE_SLAVE

    // -------------------------------------------------------------------------------

    #define CLOCK_MENU_TITLE "Miniac" 

    // -------------------------------------------------------------------------------

    // Paul's special thing
    #define COG_CRANK_OUTPUT_OFF        // COG_CRANK_OUTPUT | COG_CRANK_OUTPUT_OFF

    // -------------------------------------------------------------------------------

    // Access to Crypto/FX tickers
    #define FEATURE_TICKER_OFF             // FEATURE_TICKER | FEATURE_TICKER_OFF

    // -------------------------------------------------------------------------------
#endif
