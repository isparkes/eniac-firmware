#include "globals.h"

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

// Defined here to allow mutex on the display variables
portMUX_TYPE timerMux1 = portMUX_INITIALIZER_UNLOCKED;

int blinkState = 0;
float fadeStepsInternal = 0;

byte numberArray[DIGIT_COUNT]     = {0, 0, 0, 0, 0, 0};
byte currNumberArray[DIGIT_COUNT] = {0, 0, 0, 0, 0, 0};
byte displayType[DIGIT_COUNT]     = {NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL};
int fadeState                     = 0;
byte scrollCounter[DIGIT_COUNT]   = {0, 0, 0, 0, 0, 0};
byte valueDisplayTime             = 0;
byte valueToShow[3]               = {0, 0, 0};
byte valueDisplayType[3]          = {0x33, 0x33, 0x33}; // All normal by default

// ************************************************************
// Variables for clock management
// ************************************************************
unsigned long previousMillisWiFi = 0;
unsigned long lastMillis = 0;
unsigned long nowMillis = 0;
int lastSecond = 0;
boolean triggeredThisSec = false;

byte timeSource = TIME_SOURCE_INT;

int ldrValue;

// Menu  management
unsigned int oledTimeout = OLED_ON_TIME;
unsigned int configTimeout = 0;
unsigned int flashTimeout = 0;

// ToDo move into outputManager
bool led1State;
bool led2State;
bool indLed1;
bool indLed2;

#ifdef DIGIT_DIAGNOSTICS
// Used for testing
int digitValue = 0;
#endif

bool doAutoReconnect = false;

String uniqHostname;

// singleton object
AsyncWebServer server(80);

String lastWiFiScan = "";

void updateNowMillis() {
    nowMillis = millis();
}
