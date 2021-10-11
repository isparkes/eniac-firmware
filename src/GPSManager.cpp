#include "GPSManager.h"

// ************************************************************
// Turn the GPS string into a parsed string
// ************************************************************
String GPSManager::parseGPZDAMsg(String messageToParse) {
  if (messageToParse.length() == 36) {
    String result = messageToParse.substring(23,27) + ":" +
                    messageToParse.substring(20,22) + ":" + 
                    messageToParse.substring(17,19) + " " + 
                    messageToParse.substring(7,9) + ":" + 
                    messageToParse.substring(9,11) + ":" + 
                    messageToParse.substring(11,13);
    return result; 
  } else {
    return "";
  }
}

// ************************************************************
// Return previously calculated value
// ************************************************************
void GPSManager::calculateCurrentOffset(int year, int mon, int day, int hour, int min, int sec) {
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

// ************************************************************
// Turn the GPS string into a time_t and then onto a time string
// ************************************************************
String GPSManager::parseGPZDAMsgToLocaltime(String messageToParse) {
  if (messageToParse.length() == 36) {
    time_t tReceived;
    struct tm whenStart;
    whenStart.tm_year = messageToParse.substring(23,27).toInt() - 1900;
    whenStart.tm_mon = messageToParse.substring(20,22).toInt() - 1; 
    whenStart.tm_mday = messageToParse.substring(17,19).toInt(); 
    whenStart.tm_hour = messageToParse.substring(7,9).toInt(); 
    whenStart.tm_min = messageToParse.substring(9,11).toInt();
    whenStart.tm_sec = messageToParse.substring(11,13).toInt();

    tReceived = mktime(&whenStart) + _UTCoffset;
    const tm *tm = localtime(&tReceived);

    String timeString = String(tm->tm_year + 1900) + "," + String(tm->tm_mon + 1) + "," + String(tm->tm_mday) + "," + String(tm->tm_hour) + "," + String(tm->tm_min) + "," + String(tm->tm_sec);

    return timeString; 
  } else {
    return "";
  }
}

// ************************************************************
// Picks messages like this "$GPZDA,184937.00,28,08,2021,00,00*65"
// ************************************************************
void GPSManager::parseNMEAMsg(char c, unsigned long nowMillis) {
//  debugMsgCont("GPS: " + String(c));
  switch(c) {
    case '\r':
    case '\n':
    {
      _msgBuffer[sizeof(_msgBuffer)-1] = 0;
      String lastMessage = String(_msgBuffer);
      if (lastMessage.indexOf("$GPZDA") >= 0) {
        #ifdef DEBUG_ON 
        debugMsg("Got GPS ZDA msg: " + _lastGPSTime);
        #endif
        _lastGPSTimeRaw = lastMessage;
        _lastGPSTime = parseGPZDAMsgToLocaltime(lastMessage);
        if (_lastGPSTime != "") {
          _lastGPSReadTime = nowMillis;
        }
      }
      return;
    }
    case '$': { // sentence begin
      memset(_msgBuffer, 0, sizeof(_msgBuffer));
      _bufferOffset = 0;
      _msgBuffer[_bufferOffset++] = c;
      return;
    }
    default:
      // ordinary characters
      if (_bufferOffset < sizeof(_msgBuffer) - 1)
        _msgBuffer[_bufferOffset++] = c;
  }
}

// ************************************************************
// Return the calculated UTC offset, only available after
// calculateCurrentOffset has been callled
// ************************************************************
int  GPSManager::getCurrentUTCOffset() {
  return _UTCoffset;
}

// ************************************************************
// Output a logging message to the debug output, if set
// ************************************************************
void GPSManager::debugMsg(String message) {
  if (_dbcb != NULL && _debug) {
    _dbcb("LDR: " + message);
  }
}

// ************************************************************
// Set the callback for outputting debug messages
// ************************************************************
void GPSManager::setDebugCallback(DebugCallback dbcb) {
  _dbcb = dbcb;
  debugMsg("Debugging started, callback set");
}

// ************************************************************
// set the update interval
// ************************************************************
void GPSManager::setDebugOutput(bool newDebug) {
  _debug = newDebug;
}

// ************************************************************
// Get if we are still in the GPS valid time
// ************************************************************
bool GPSManager::getGPSTimeValid(unsigned long nowMillis) {
  if (_lastGPSReadTime > 0) {
    return ((nowMillis - _lastGPSReadTime)/1000 < GPS_READING_VALIDITY_SECS);
  } else {
    return false;
  }
}

// ************************************************************
// The last time we last got a GPS update
// ************************************************************
unsigned long GPSManager::getLastGPSReadTime() {
  return _lastGPSReadTime;
}

// ************************************************************
// Get the last GPS time we read
// ************************************************************
String GPSManager::getLastGPSTime() {
  return _lastGPSTime;
}

// ************************************************************
// Get the last raw GPS time string we read
// ************************************************************
String GPSManager::getLastGPSTimeRaw() {
  return _lastGPSTimeRaw;
}

// ************************************************************
// Get the singleton instance
// ************************************************************
GPSManager &GPSManager::getInstance() {
  static GPSManager instance;
  return instance;
}

GPSManager &gpsManager = gpsManager.getInstance();
