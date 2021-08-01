#include "globals.h"
#include "defs.h"

byte numberArray[DIGIT_COUNT];

SPIFFS_CLOCK spiffsStorage;
spiffs_config_t current_config;
spiffs_stats_t current_stats;

spiffs_config_t* cc = &current_config;
spiffs_stats_t* cs = &current_stats;

AsyncWebServer server(80);

NtpAsync ntpAsync;

volatile uint32_t val1 = 0;
volatile uint32_t val2 = 0;
volatile uint32_t val3 = 0;

unsigned long previousMillisWiFi = 0;
unsigned long lastMillis = 0;
unsigned long nowMillis = 0;
int lastSecond = 0;
boolean triggeredThisSec = false;

int ldrValue;

String ssid = "";
String password = "";
bool credentialsReceived = false;

