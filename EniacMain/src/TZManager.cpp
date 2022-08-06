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
  debugMsgTzm("Set TZS: " + tzs);
  _tzs = tzs;
  setenv("TZ", _tzs.c_str(), 1);
}

// ************************************************************
// Calculate the offset from the time we received
// ************************************************************
void TZManager_::calculateCurrentOffsetFromTimeT() {
  time_t primarytime_t = getRawUTCTimeFromTimeSource(_primarysource);
  debugMsgTzm("Offset Input: " + String(primarytime_t) + " from primary source " + String(_primarysource) + " U--> " + tzManager.gmtimeToReadableString(primarytime_t));

  struct tm info_local;
  struct tm info_gm;
  localtime_r(&primarytime_t, &info_local);
  gmtime_r(&primarytime_t, &info_gm);

  debugMsgTzm("local L--> " + localtimeToReadableString(primarytime_t));
  debugMsgTzm("gm U--> " + gmtimeToReadableString(primarytime_t));

  // The local time might be in DST, so correct that
  info_gm.tm_isdst = 0;
  info_local.tm_isdst = 0;

  _UTCoffset = mktime(&info_local) - mktime(&info_gm);
  _localtimeisDST = info_local.tm_isdst;

  debugMsgTzm("UTC offset: " + String(_UTCoffset));
  debugMsgTzm("localtime is in DST: " + String(_localtimeisDST));
}

// ************************************************************
// get the currend UTC offset
// ************************************************************
int TZManager_::getCurrentUTCOffset() {
  return (int)_UTCoffset;
}

// ************************************************************
// get the currend UTC offset
// ************************************************************
int TZManager_::getCurrentUTCIsDST() {
  return (int)_localtimeisDST;
}

// ************************************************************
// Update the UTC value from this time source. If NTP, then
// we update the RTC time and internal time to the value.
// ************************************************************
void TZManager_::setUTCTimeFromTimeSource(byte timesource, unsigned long readTime, time_t utcTime) {
  if (timesource == _primarysource) {
    debugMsgTzm("Update PRIMARY time from source " + String(timesource) + ": " + String(utcTime) + " at millis: " + String(readTime));
  } else {
    debugMsgTzm("Update time from source " + String(timesource) + ": " + String(utcTime) + " at millis: " + String(readTime));
  }

  // Because GPS gives us an update every second, we don't want to set the RTC all the time
  // instead we want to update the RTC only in the case that it has not been set (e.g. at startup)
  // or that we haven't updated for a time
  if (timesource == TIME_SOURCE_GPS) {

    _utctime[timesource] = utcTime;
    _lastupdatetime[timesource] = readTime;

    unsigned long lastRTCUpdate = getTimeLastSetFromTimeSource(TIME_SOURCE_RTC);
    if ((lastRTCUpdate > RTC_WRITE_CACHE_TIME_SEC) || // The last update is old
      (_lastupdatetime[TIME_SOURCE_RTC] == 0)) {      // or the RTC was not yet initialised
        // Update the RTC time
        setRTCTimeFromTimeSource(readTime, utcTime);
        
        // Set the internal time
        setInternalTimeFromRTC();
      }
  } else if (timesource == TIME_SOURCE_NTP) {

    _utctime[timesource] = utcTime;
    _lastupdatetime[timesource] = readTime;

    // Update the RTC time
    setRTCTimeFromTimeSource(readTime, utcTime);

    // Set the internal time
    setInternalTimeFromRTC();
  } else if (timesource == TIME_SOURCE_RTC) {
    // initialise at start up
    if (_lastupdatetime[TIME_SOURCE_RTC] == 0) {
      debugMsgTzm("Setting BOOT time for time source 2: U-->" + String(utcTime) + " at millis: 1");
      _utctime[TIME_SOURCE_RTC] = utcTime;
      _lastupdatetime[TIME_SOURCE_RTC] = 1;
    } 
    // Set the internal time
    setInternalTimeFromRTC();
  }
}

// ************************************************************
// Update the RTC time
// ************************************************************
void TZManager_::setRTCTimeFromTimeSource(unsigned long readTime, time_t utcTime) {
  debugMsgTzm("Set RTC time to time " + String(utcTime));
  rtcManager.setTimeFromUTCSource(utcTime, true);
  _utctime[TIME_SOURCE_RTC] = utcTime;
  _lastupdatetime[TIME_SOURCE_RTC] = readTime;
}

// ************************************************************
// Update the UTC value from the GPS time source. We don't do 
// this on event, because the events gome once per second, and
// that is too often.
// ************************************************************
void TZManager_::setUTCTimeFromTimeSourceHourly() {
  if (_primarysource == TIME_SOURCE_GPS) {
    unsigned long offset = getTimeLastSetFromTimeSource(TIME_SOURCE_GPS);
    if (offset < GPS_READING_VALIDITY_SECS) {
      time_t nowtime_t = _utctime[TIME_SOURCE_GPS] + offset;

      rtcManager.setTimeFromUTCSource(nowtime_t, true);
      _utctime[TIME_SOURCE_RTC] = nowtime_t;
      _lastupdatetime[TIME_SOURCE_RTC] = nowMillis;
      setInternalTimeFromRTC();
      debugMsgTzm("Set RTC time to timesource " + String(TIME_SOURCE_GPS) + " time " + String(nowtime_t));
    } else {
      debugMsgTzm("GPS is not current, nothing to do");
    }
  } else {
    debugMsgTzm("PRIMARY time source is not GPS, nothing to do");
  }
}

// ************************************************************
// sets the internal time to local time from the RTC
// ************************************************************
void TZManager_::setInternalTimeFromRTC() {
  int lastUpdateOffset = getTimeLastSetFromTimeSource(TIME_SOURCE_RTC);
  time_t currentUTC = _utctime[TIME_SOURCE_RTC] + lastUpdateOffset;
  debugMsgTzm("Recovered current UTC from RTC " + String(currentUTC));
  struct tm info_local;
  localtime_r(&currentUTC, &info_local);

  #ifdef TZM_EXTENDED_DEBUG
  debugMsgTzm("---->Converted to " + String(info_local.tm_year + 1900) + "," + String(info_local.tm_mon + 1) + "," + String(info_local.tm_mday) + "," + String(info_local.tm_hour) + "," + String(info_local.tm_min) + "," + String(info_local.tm_sec));
  #endif

  setTime(info_local.tm_hour,
    info_local.tm_min,
    info_local.tm_sec,
    info_local.tm_mday,
    info_local.tm_mon + 1, 
    info_local.tm_year + 1900);

  #ifdef TZM_EXTENDED_DEBUG
  debugMsgTzm("---->RTC Offset: " + String(lastUpdateOffset));
  #endif
  debugMsgTzm("Set internal time to timesource " + String(TIME_SOURCE_RTC) + ": " + String(_utctime[TIME_SOURCE_RTC]) + " L--> " + localtimeToReadableString(_utctime[TIME_SOURCE_RTC]));
  _utctime[TIME_SOURCE_INT] = _utctime[TIME_SOURCE_RTC] + lastUpdateOffset;
  _lastupdatetime[TIME_SOURCE_INT] = nowMillis;
}

// ************************************************************
// gets the local time from the RTC
// ************************************************************
tm TZManager_::getRTCTimeAsLocalTimeTM() {
  int lastUpdateOffset = getTimeLastSetFromTimeSource(TIME_SOURCE_RTC);
  time_t currentUTC = _utctime[TIME_SOURCE_RTC] + lastUpdateOffset;
  debugMsgTzm("Recovered current UTC from RTC " + String(currentUTC));
  struct tm info_local;
  localtime_r(&currentUTC, &info_local);
  return info_local;
}

// ************************************************************
// Convert a tm struct to a UTC time_t
// ************************************************************
time_t TZManager_::convertLocalTimeTMToUTC(tm tmFrom) {
  #ifdef TZM_EXTENDED_DEBUG
  debugMsgTzm("---->Applying offset: " + String(getCurrentUTCOffset()) + " to RTC harware time -> " + 
    String(tmFrom.tm_year) + ", " + String(tmFrom.tm_mon) + ", " + String(tmFrom.tm_mday) + ", " + 
    String(tmFrom.tm_hour) + ", " + String(tmFrom.tm_min) + ", " + String(tmFrom.tm_sec));
  #endif

  time_t _rtctime = mktime(&tmFrom);
  return _rtctime;
}

// ************************************************************
// get the estimated local time from a time source 
// ************************************************************
String TZManager_::getLocalTimeFromTimeSource(byte timesource) {
  if(_lastupdatetime[timesource] == 0) {
    debugMsgTzm("Timesource: " + String(timesource) + ": No data");
    return "Unknown";
  }

  unsigned long offset = getTimeLastSetFromTimeSource(timesource);
  time_t nowtime_t = _utctime[timesource] + offset;

  // If we are talking to the RTC, every now and again update the internal time
  if (timesource == TIME_SOURCE_INT && (offset >= RTC_READ_CACHE_TIME_SEC)) {
    debugMsgTzm("Internal time is : " + String(offset) + "S old, getting update");
    setInternalTimeFromRTC();
  }

  String formattedTime = localtimeToReadableString(nowtime_t);
  #ifdef DEBUG_ON
  String primary = "";
  if (timesource == _primarysource) {
    primary = " *";
  }
  debugMsgTzm("Timesource: " + String(timesource) + ": " + String(_utctime[timesource]) + " offset " + String(offset) + " L--> " + formattedTime + primary);
  #endif

  return formattedTime;
}

// ************************************************************
// get the estimated epoch time from a time source 
// ************************************************************
time_t TZManager_::getRawUTCTimeFromTimeSource(byte timesource) {
  if(_lastupdatetime[timesource] == 0) {
    debugMsgTzm("Timesource: " + String(timesource) + ": No data");
    return 0;
  }

  unsigned long offset = getTimeLastSetFromTimeSource(timesource);
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
  if (nowMillis < _lastupdatetime[timesource]) {
    debugMsgTzm("!!!!!!Negative offset for timesource: " + String(timesource) + "!!!");
    offset = 0;
  }
  #ifdef TZM_EXTENDED_DEBUG
  debugMsgTzm("---->Time last set = " + String(offset) + " for time source " + String(timesource));
  debugMsgTzm("---->nowmillis = " + String(nowMillis) + " lastupdate " + String(_lastupdatetime[timesource]));
  #endif
  return offset;
}

// ************************************************************
// select the most reliable time source
// Should be called once per minute, sets UTC offset
// ************************************************************
byte TZManager_::getPrimaryTimeSource() {
  #ifdef TZM_EXTENDED_DEBUG
  debugMsgTzm("---->Summary");
  debugMsgTzm("---->GPS: " + String(_utctime[TIME_SOURCE_GPS]) + "@" + String(_lastupdatetime[TIME_SOURCE_GPS]) + " L--> " + localtimeToReadableString(_utctime[TIME_SOURCE_GPS]));
  debugMsgTzm("---->NTP: " + String(_utctime[TIME_SOURCE_NTP]) + "@" + String(_lastupdatetime[TIME_SOURCE_NTP]) + " L--> " + localtimeToReadableString(_utctime[TIME_SOURCE_NTP]));
  debugMsgTzm("---->RTC: " + String(_utctime[TIME_SOURCE_RTC]) + "@" + String(_lastupdatetime[TIME_SOURCE_RTC]) + " L--> " + localtimeToReadableString(_utctime[TIME_SOURCE_RTC]));
  debugMsgTzm("---->INT: " + String(_utctime[TIME_SOURCE_INT]) + "@" + String(_lastupdatetime[TIME_SOURCE_INT]) + " L--> " + localtimeToReadableString(_utctime[TIME_SOURCE_INT]));
  #endif
  byte oldPrimary = _primarysource;
  if (_lastupdatetime[TIME_SOURCE_GPS] != 0 && getTimeLastSetFromTimeSource(TIME_SOURCE_GPS) < GPS_READING_VALIDITY_SECS) {
    _primarysource = TIME_SOURCE_GPS;
  } else if (_lastupdatetime[TIME_SOURCE_NTP] != 0 && ntpManager.ntpTimeValid()) {
    _primarysource = TIME_SOURCE_NTP;
  } else if (rtcManager.getRTCValid()) {
    _primarysource = TIME_SOURCE_RTC;
  } else {
    _primarysource = TIME_SOURCE_INT;
  }

  if (oldPrimary != _primarysource) {
    debugMsgTzm("Changed primary time source, old: " + String(oldPrimary) + ", new: " + String(_primarysource));
  }
  calculateCurrentOffsetFromTimeT();

  return _primarysource;
}

TZManager_ &TZManager_::getInstance() {
  static TZManager_ instance;
  return instance;
}

TZManager_ &tzManager = tzManager.getInstance();