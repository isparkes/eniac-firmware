#include "Globals.h"

// ************************************************************
// Global shared components and objects
// ************************************************************
esp_wps_config_t wps_config;

// ************************************************************
// Shared config objects
// ************************************************************
spiffs_config_t current_config;
spiffs_stats_t current_stats;

spiffs_config_t* cc = &current_config;
spiffs_stats_t* cs = &current_stats;

// ************************************************************
// Display values
// ************************************************************
volatile uint32_t val1 = 0;
volatile uint32_t val2 = 0;
volatile uint32_t val3 = 0;

volatile uint32_t nextVal1 = 0x81;
volatile uint32_t nextVal2 = 0x81;
volatile uint32_t nextVal3 = 0x81;

volatile uint8_t switchTime = 0;

volatile uint16_t impressions;

// Makes sure we are not writing to the display buffer
// when the interrupt is reading it
portMUX_TYPE timerMux1 = portMUX_INITIALIZER_UNLOCKED;

// ************************************************************
// Variables for clock management
// ************************************************************
unsigned long nowMillis = 0;
unsigned long previousMillisWiFi = 0;
unsigned long lastSecondStartMillis = 0;
int lastSecond = 0;
boolean triggeredThisSec = false;
int secsDeltaAbs;                   // Sawtooth 0..1000 every sec
int secsDelta;                      // Triangle 0..1000..0 every 2 secs
bool upOrDown;  

// Current normalised LDR value
int ldrValue;

// Menu  management - OLED timeouts
unsigned int oledTimeout = OLED_ON_TIME;
unsigned int configTimeout = 0;
unsigned int flashTimeout = 0;

#ifdef DIGIT_DIAGNOSTICS
int digitValue = 0;
#endif

bool doAutoReconnect = false;

// Our network name
String uniqHostname;

// singleton object
AsyncWebServer server(80);

String lastWiFiScan = "";

// We don't want to handle switch processing in an interrupt,
// so we set a flag to trigger processing
// Set true to trigger initialisation
bool switchEventWaiting = true;

// ------------------ Global functions -----------------

void updateNowMillis() {
    nowMillis = millis();
}

#ifdef COG_CRANK_OUTPUT
byte cogCrankSecsLeft = 0;
#endif
