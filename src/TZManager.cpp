#include "TZManager.h"
#include <Arduino.h>

// ************************************************************
// Return previously calculated value
// ************************************************************
void TZManager_::calculateCurrentOffset(int year, int mon, int day, int hour, int min, int sec) {
    struct tm whenStart;
    whenStart.tm_year = year - 1900;
    whenStart.tm_mon = mon - 1; 
    whenStart.tm_mday = day; 
    whenStart.tm_hour = hour; 
    whenStart.tm_min = min;
    whenStart.tm_sec = sec;

    time_t now = mktime(&whenStart);

    #ifdef DEBUG_ON
    const char *str = ctime(&now);
    debugMsg("input: " + String(str));
    #endif

    struct tm info_local;
    struct tm info_gm;
    localtime_r(&now, &info_local);
    gmtime_r(&now, &info_gm);

    #ifdef DEBUG_ON
    String timeStringLocal = String(info_local.tm_year + 1900) + "," + String(info_local.tm_mon + 1) + "," + String(info_local.tm_mday) + "," + String(info_local.tm_hour) + "," + String(info_local.tm_min) + "," + String(info_local.tm_sec);
    String timeStringGm = String(info_gm.tm_year + 1900) + "," + String(info_gm.tm_mon + 1) + "," + String(info_gm.tm_mday) + "," + String(info_gm.tm_hour) + "," + String(info_gm.tm_min) + "," + String(info_gm.tm_sec);

    debugMsg("local: " + timeStringLocal);
    debugMsg("gm: " + timeStringGm);
    #endif

    // The local time might be in DST, so correct that
    info_gm.tm_isdst = 0;
    info_local.tm_isdst = 0;

    _UTCoffset = mktime(&info_local) - mktime(&info_gm);

    #ifdef DEBUG_ON
    debugMsg("UTC offset: " + String(_UTCoffset));
    #endif
}

void TZManager_::begin() {
  pinMode(LED_BUILTIN, OUTPUT);

  // blink the led a few times
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
  }

  digitalWrite(LED_BUILTIN, LOW);
}

int TZManager_::getCurrentUTCOffset() {
  return (int)_UTCoffset;
}

TZManager_ &TZManager_::getInstance() {
  static TZManager_ instance;
  return instance;
}

TZManager_ &tzManager = tzManager.getInstance();