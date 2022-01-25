// ************************************************************
// Time Zone Manager acts as the central location for all time
// sources. There is a priority of importance. First GPS is 
// considered the best, after that, NTP is preferred.
// The internal RTC is used as the long term time source if 
// neither GPS nor NTP is available.
//
// The RTC keeps the internal controller time up to date, and
// all local work in the code uses the internal time.
// If an update comes from GPS or NTP, this resets both the 
// RTC and the internal variables.
// ************************************************************

#include "TZManager.h"
#include <Arduino.h>
#include "defs.h"
#include "utilities.h"

// Suppress Intellisense "setenv" error
_VOID      _EXFUN(tzset,	(_VOID));
int	_EXFUN(setenv,(const char *__string, const char *__value, int __overwrite));

// ************************************************************
// Time Zone String getter/setter
// ************************************************************
String TZManager_::getTZS() {
  return _tzs;
}

void TZManager_::setTZS(String tzs) {
  #ifdef DEBUG_ON
  debugMsg("Set TZS: " + tzs);
  #endif
  _tzs = tzs;
  setenv("TZ", _tzs.c_str(), 1);
}

// ************************************************************
// Calculate the offset from the time we received
// ************************************************************
void TZManager_::calculateCurrentOffsetFromTimeT() {
  time_t primarytime_t = _utctime[_primarysource];
    #ifdef DEBUG_ON
    const char *str = ctime(&primarytime_t);
    debugMsg("input: " + String(primarytime_t) + " from primary source " + String(_primarysource) + " time " + str);
    #endif

    struct tm info_local;
    struct tm info_gm;
    localtime_r(&primarytime_t, &info_local);
    gmtime_r(&primarytime_t, &info_gm);

    #ifdef DEBUG_ON
    String timeStringLocal = String(info_local.tm_year + 1900) + "-" + String(info_local.tm_mon + 1) + "-" + String(info_local.tm_mday) + " " + String(info_local.tm_hour) + ":" + String(info_local.tm_min) + ":" + String(info_local.tm_sec);
    String timeStringGm = String(info_gm.tm_year + 1900) + "-" + String(info_gm.tm_mon + 1) + "-" + String(info_gm.tm_mday) + " " + String(info_gm.tm_hour) + ":" + String(info_gm.tm_min) + ":" + String(info_gm.tm_sec);

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

// ************************************************************
// get the currend UTC offset
// ************************************************************
int TZManager_::getCurrentUTCOffset() {
  return (int)_UTCoffset;
}

// ************************************************************
// Update the UTC value from this time source
// ************************************************************
void TZManager_::setUTCTimeFromTimeSource(byte timesource, unsigned long now, time_t utcTime) {
  _utctime[timesource] = utcTime;
  _lastupdatetime[timesource] = now;

  #ifdef DEBUG_ON
  if (_primarysource) {
    debugMsg("Update PRIMARY time from source " + String(timesource) + ": " + String(utcTime) + " at millis: " + String(now));
  } else {
    debugMsg("Update time from source " + String(timesource) + ": " + String(utcTime) + " at millis: " + String(now));
  }
  #endif

  if (timesource < TIME_SOURCE_RTC && timesource == _primarysource) {
    #ifdef DEBUG_ON
      debugMsg("Set RTC time to timesource " + String(timesource) + " time " + String(utcTime));
    #endif
    rtcManager.setTimeFromUTCSource(utcTime, true);
    setInternalTime();
  } 
}

// ************************************************************
// sets the internal time to local time from the RTC
// ************************************************************
void TZManager_::setInternalTime() {
  time_t currentUTC = _utctime[TIME_SOURCE_RTC];
  struct tm info_local;
  localtime_r(&currentUTC, &info_local);

  setTime(info_local.tm_hour,
    info_local.tm_min,
    info_local.tm_sec,
    info_local.tm_mday,
    info_local.tm_mon + 1, 
    info_local.tm_year + 1900);

  #ifdef DEBUG_ON
    debugMsg("Set internal time to timesource " + String(TIME_SOURCE_RTC) + " time " + String(_utctime[TIME_SOURCE_RTC]));
  #endif
}

// ************************************************************
// get the estimated local time from this time source 
// ************************************************************
String TZManager_::getLocalTimeFromTimeSource(byte timesource, unsigned long now) {
  if(_lastupdatetime[timesource] == 0) {
    return "Unknown";
  }

  unsigned long offset = (now - _lastupdatetime[timesource])/1000;
  time_t nowtime_t = _utctime[timesource] + offset;

  #ifdef DEBUG_ON
  debugMsg("Timesource: " + String(timesource) + " UTC " + String(_utctime[timesource]) + " offset " + String(offset));
  #endif

  struct tm info_local;
  localtime_r(&nowtime_t, &info_local);

  String formattedTime = timeToReadableStringFromTm(info_local);
  return formattedTime;
}

// ************************************************************
// Return how long ago we heard from this source
// ************************************************************
unsigned long TZManager_::getTimeLastSetFromTimeSource(byte timesource, unsigned long now) {
  unsigned long offset = (now - _lastupdatetime[timesource])/1000;
  return offset;
}

// ************************************************************
// select the most reliable time source
// Should be called once per minute, sets UTC offset
// ************************************************************
byte TZManager_::getPrimaryTimeSource(unsigned long now) {
  #ifdef DEBUG_ON
  byte oldPrimary = _primarysource;
  #endif
  if (_lastupdatetime[TIME_SOURCE_GPS] != 0 && getTimeLastSetFromTimeSource(TIME_SOURCE_GPS, now) < GPS_READING_VALIDITY_SECS) {
    _primarysource = TIME_SOURCE_GPS;
  } else if (_lastupdatetime[TIME_SOURCE_NTP] != 0 && ntpManager.ntpTimeValid(now)) {
    _primarysource = TIME_SOURCE_NTP;
  } else if (rtcManager.getRTCValid()) {
    _primarysource = TIME_SOURCE_RTC;
  } else {
    _primarysource = TIME_SOURCE_INT;
  }

  #ifdef DEBUG_ON
  if (oldPrimary != _primarysource) {
    debugMsg("Changed primary time source, old: " + String(oldPrimary) + ", new: " + String(_primarysource));
  }
  #endif
  calculateCurrentOffsetFromTimeT();

  return _primarysource;
}

// ************************************************************
// set the update interval
// ************************************************************
void TZManager_::setDebugOutput(bool newDebug) {
  _debug = newDebug;
}

// ************************************************************
// Output a logging message to the debug output, if set
// ************************************************************
void TZManager_::debugMsg(String message) {
  if (_dbcb != NULL && _debug) {
    _dbcb("[TZM]: " + message);
  }
}

// ************************************************************
// Set the callback for outputting debug messages
// ************************************************************
void TZManager_::setDebugCallback(DebugCallback dbcb) {
  _dbcb = dbcb;
  #ifdef DEBUG_ON
  debugMsg("Debugging started, callback set");
  #endif
}

TZManager_ &TZManager_::getInstance() {
  static TZManager_ instance;
  return instance;
}

TZManager_ &tzManager = tzManager.getInstance();