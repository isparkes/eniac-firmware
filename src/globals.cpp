#include "globals.h"

byte numberArray[DIGIT_COUNT];

// ************************************************************
// SPIFFS and public configs
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

DS1307 rtclock;

// ************************************************************
// SPIFFS and public configs
// ************************************************************
volatile uint32_t val1 = 0;
volatile uint32_t val2 = 0;
volatile uint32_t val3 = 0;

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

bool blanked;
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

bool useRTC;

String lastGPSTime = "";
unsigned long lastGPSReadTime = 0;
bool gpsTimeValid = false;