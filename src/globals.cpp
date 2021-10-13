#include "globals.h"
#include "defs.h"

// ************************************************************
// Global shared components and objects
// ************************************************************
SPIFFS_CLOCK spiffsStorage;
spiffs_config_t current_config;
spiffs_stats_t current_stats;

spiffs_config_t* cc = &current_config;
spiffs_stats_t* cs = &current_stats;

AsyncWebServer server(80);

NtpAsync ntpAsync;

esp_wps_config_t wps_config;

ESP32Encoder encoder;

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
// SPIFFS and public configs
// ************************************************************
unsigned long previousMillisWiFi = 0;
unsigned long lastMillis = 0;
unsigned long nowMillis = 0;
int lastSecond = 0;
boolean triggeredThisSec = false;

int ldrValue;

String ssid = "";
String password = "";
bool credentialsReceived = false;

unsigned int oledTime;

// ToDo move into outputManager
bool led1State;
bool led2State;
bool indLed1;
bool indLed2;

// ************************************************************
// Blinkenlights
// ************************************************************
bool bl1;
bool bl2;
bool bl3;
bool bl4;
bool bl5;
bool bl6;