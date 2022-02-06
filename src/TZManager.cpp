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
//
// If an update comes from GPS and GPS is the primary time
// source, we update the RTC once per hours as long as the GPS
// signal is still considered valid (set by the variable
//  GPS_READING_VALIDITY_SECS).
// 
// If we get an update from NTP and NTP is the primary time
// source, we set the RTC every update, given that it is less
// frequent.
//
// We read the RTC every minute, and set the internal time
// (in local time - all other readings are kept in UTC).
//
// Clock display is taken from the internal time.
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
  time_t primarytime_t = getRawUTCTimeFromTimeSource(_primarysource);
  #ifdef DEBUG_ON
  debugMsg("Input: " + String(primarytime_t) + " from primary source " + String(_primarysource) + " U--> " + tzManager.gmtimeToReadableString(primarytime_t));
  #endif

  struct tm info_local;
  struct tm info_gm;
  localtime_r(&primarytime_t, &info_local);
  gmtime_r(&primarytime_t, &info_gm);

  #ifdef DEBUG_ON
  debugMsg("local L--> " + localtimeToReadableString(primarytime_t));
  debugMsg("gm U--> " + gmtimeToReadableString(primarytime_t));
  #endif

  // The local time might be in DST, so correct that
  info_gm.tm_isdst = 0;
  info_local.tm_isdst = 0;

  _UTCoffset = mktime(&info_local) - mktime(&info_gm);

//    int _localtimeisDST = info_local.tm_isdst;

  #ifdef DEBUG_ON
  debugMsg("UTC offset: " + String(_UTCoffset));
//    debugMsg("localtime is in DST: " + String(_localtimeisDST));
  #endif
}

// ************************************************************
// get the currend UTC offset
// ************************************************************
int TZManager_::getCurrentUTCOffset() {
  return (int)_UTCoffset;
}

// ************************************************************
// Update the UTC value from this time source. If NTP, then
// we update the RTC time and internal time to the value.
// ************************************************************
void TZManager_::setUTCTimeFromTimeSource(byte timesource, unsigned long readTime, time_t utcTime) {
  _utctime[timesource] = utcTime;
  _lastupdatetime[timesource] = readTime;

  #ifdef DEBUG_ON
  if (timesource == _primarysource) {
    debugMsg("Update PRIMARY time from source " + String(timesource) + ": " + String(utcTime) + " at millis: " + String(readTime));
  } else {
    debugMsg("Update time from source " + String(timesource) + ": " + String(utcTime) + " at millis: " + String(readTime));
  }
  #endif

  int lastRTCUpdate = getTimeLastSetFromTimeSource(TIME_SOURCE_RTC);
  if ((timesource < TIME_SOURCE_RTC) &&             // Information from a better source
      ((lastRTCUpdate > 60) ||                      // The last update is old
      (_lastupdatetime[TIME_SOURCE_RTC] == 0))) {   // or the RTC was not yet initialised
    #ifdef DEBUG_ON
      debugMsg("Set RTC time to timesource " + String(timesource) + " time " + String(utcTime));
    #endif
    rtcManager.setTimeFromUTCSource(utcTime, true);
    _utctime[TIME_SOURCE_RTC] = utcTime;
    _lastupdatetime[TIME_SOURCE_RTC] = readTime;
    setInternalTime();
  }
}

// ************************************************************
// Update the UTC value from the GPS time source. We don't do 
// this on event, because the events gome once per second, and
// that is too often.
// ************************************************************
void TZManager_::setUTCTimeFromTimeSourceHourly() {
  if (_primarysource == TIME_SOURCE_GPS) {
    unsigned long offset = (nowMillis - _lastupdatetime[TIME_SOURCE_GPS])/1000;
    if (offset < GPS_READING_VALIDITY_SECS) {
      time_t nowtime_t = _utctime[TIME_SOURCE_GPS] + offset;

      rtcManager.setTimeFromUTCSource(nowtime_t, true);
      _utctime[TIME_SOURCE_RTC] = nowtime_t;
      _lastupdatetime[TIME_SOURCE_RTC] = nowMillis;
      setInternalTime();
      #ifdef DEBUG_ON
        debugMsg("Set RTC time to timesource " + String(TIME_SOURCE_GPS) + " time " + String(nowtime_t));
      #endif
    } else {
      #ifdef DEBUG_ON
      debugMsg("GPS is not current, nothing to do");
      #endif
    }
  } else {
    #ifdef DEBUG_ON
    debugMsg("PRIMARY time source is not GPS, nothing to do");
    #endif
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

  int lastUpdateOffset = getTimeLastSetFromTimeSource(TIME_SOURCE_RTC);
  #ifdef DEBUG_ON
//  debugMsg("RTC Offset: " + String(lastUpdateOffset));
  debugMsg("Set internal time to timesource " + String(TIME_SOURCE_RTC) + ": " + String(_utctime[TIME_SOURCE_RTC]) + " L--> " + localtimeToReadableString(_utctime[TIME_SOURCE_RTC]));
  #endif
  _utctime[TIME_SOURCE_INT] = _utctime[TIME_SOURCE_RTC] + lastUpdateOffset;
  _lastupdatetime[TIME_SOURCE_INT] = nowMillis;

}

// ************************************************************
// get the estimated local time from a time source 
// ************************************************************
String TZManager_::getLocalTimeFromTimeSource(byte timesource) {
  if(_lastupdatetime[timesource] == 0) {
    #ifdef DEBUG_ON
    debugMsg("Timesource: " + String(timesource) + ": No data");
    #endif
    return "Unknown";
  }

  unsigned long offset = (nowMillis - _lastupdatetime[timesource])/1000;
  time_t nowtime_t = _utctime[timesource] + offset;

  // If we are talking to the RTC, every now and again update the internal time
  if (timesource == TIME_SOURCE_INT && (offset >= RTC_CACHE_TIME_SEC)) {
    #ifdef DEBUG_ON
    debugMsg("Internal time is : " + String(offset) + "S old, getting update");
    #endif
    setInternalTime();
  }

  String formattedTime = localtimeToReadableString(nowtime_t);
  #ifdef DEBUG_ON
  String primary = "";
  if (timesource == _primarysource) {
    primary = " *";
  }
  debugMsg("Timesource: " + String(timesource) + ": " + String(_utctime[timesource]) + " offset " + String(offset) + " L--> " + formattedTime + primary);
  #endif

  return formattedTime;
}

// ************************************************************
// get the estimated epoch time from a time source 
// ************************************************************
time_t TZManager_::getRawUTCTimeFromTimeSource(byte timesource) {
  if(_lastupdatetime[timesource] == 0) {
    #ifdef DEBUG_ON
    debugMsg("Timesource: " + String(timesource) + ": No data");
    #endif
    return 0;
  }

  unsigned long offset = (nowMillis - _lastupdatetime[timesource])/1000;
  time_t nowtime_t = _utctime[timesource] + offset;

  return nowtime_t;
}

// ************************************************************
// Format a time_t as a localtime string  
// ************************************************************
String TZManager_::localtimeToReadableString(time_t timeToConvert) {
  struct tm info_local;
  localtime_r(&timeToConvert, &info_local);

  String formattedTime = timeToReadableStringFromTm(info_local);
  return formattedTime;
}

// ************************************************************
// Format a time_t as a gmtime string  
// ************************************************************
String TZManager_::gmtimeToReadableString(time_t timeToConvert) {
  struct tm info_local;
  gmtime_r(&timeToConvert, &info_local);

  String formattedTime = timeToReadableStringFromTm(info_local);
  return formattedTime;
}

// ************************************************************
// Return how long ago we heard from this source
// ************************************************************
unsigned long TZManager_::getTimeLastSetFromTimeSource(byte timesource) {
  unsigned long offset = (nowMillis - _lastupdatetime[timesource])/1000;
  return offset;
}

// ************************************************************
// select the most reliable time source
// Should be called once per minute, sets UTC offset
// ************************************************************
byte TZManager_::getPrimaryTimeSource() {
  #ifdef DEBUG_ON
  byte oldPrimary = _primarysource;
  #endif
  if (_lastupdatetime[TIME_SOURCE_GPS] != 0 && getTimeLastSetFromTimeSource(TIME_SOURCE_GPS) < GPS_READING_VALIDITY_SECS) {
    _primarysource = TIME_SOURCE_GPS;
  } else if (_lastupdatetime[TIME_SOURCE_NTP] != 0 && ntpManager.ntpTimeValid()) {
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