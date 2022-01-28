#include "GPSManager.h"

// ************************************************************
// Turn the GPS string into a parsed string
// ************************************************************
String GPSManager_::parseGPZDAMsg(String messageToParse) {
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
// Turn the GPS string into a time_t and then onto a time string
// ************************************************************
bool GPSManager_::parseGPZDAMsgToUTCTime(String messageToParse, unsigned long nowMillis) {
  if (messageToParse.length() == 36) {
    time_t tReceived;
    struct tm whenStart;
    whenStart.tm_year = messageToParse.substring(23,27).toInt() - 1900;
    whenStart.tm_mon = messageToParse.substring(20,22).toInt() - 1; 
    whenStart.tm_mday = messageToParse.substring(17,19).toInt(); 
    whenStart.tm_hour = messageToParse.substring(7,9).toInt(); 
    whenStart.tm_min = messageToParse.substring(9,11).toInt();
    whenStart.tm_sec = messageToParse.substring(11,13).toInt();

    tReceived = mktime(&whenStart) + tzManager.getCurrentUTCOffset();

    #ifdef DEBUG_ON
    
    debugMsg("Received GPS update U--> " + tzManager.gmtimeToReadableString(tReceived));
    #endif

    tzManager.setUTCTimeFromTimeSource(TIME_SOURCE_GPS, nowMillis, tReceived);

    return true;
  } else {
    return false;
  }
}

// ************************************************************
// Picks messages like this "$GPZDA,184937.00,28,08,2021,00,00*65"
// ************************************************************
void GPSManager_::parseNMEAMsg(char c, unsigned long nowMillis) {
//  debugMsgCont("GPS: " + String(c));
  switch(c) {
    case '\r':
    case '\n':
    {
      _msgBuffer[sizeof(_msgBuffer)-1] = 0;
      String lastMessage = String(_msgBuffer);
      if (lastMessage.indexOf("$GPZDA") >= 0) {
        _lastGPSTimeRaw = lastMessage;
        _lastGPSSyncTime = nowMillis;
        #ifdef DEBUG_ON 
        debugMsg("Got GPS ZDA msg: " + lastMessage);
        #endif
        if(parseGPZDAMsgToUTCTime(lastMessage, nowMillis)) {
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
// Output a logging message to the debug output, if set
// ************************************************************
void GPSManager_::debugMsg(String message) {
  if (_dbcb != NULL && _debug) {
    _dbcb("[GPS]: " + message);
  }
}

// ************************************************************
// Set the callback for outputting debug messages
// ************************************************************
void GPSManager_::setDebugCallback(DebugCallback dbcb) {
  _dbcb = dbcb;
  debugMsg("Debugging started, callback set");
}

// ************************************************************
// set the update interval
// ************************************************************
void GPSManager_::setDebugOutput(bool newDebug) {
  _debug = newDebug;
}

// ************************************************************
// Get if we are still in the GPS valid time
// ************************************************************
bool GPSManager_::getGPSTimeValid(unsigned long nowMillis) {
  if (_lastGPSReadTime > 0) {
    return ((nowMillis - _lastGPSReadTime)/1000 < GPS_READING_VALIDITY_SECS);
  } else {
    return false;
  }
}

// ************************************************************
// Get if we have started to synch, but are not yet ready
// ************************************************************
bool GPSManager_::getGPSSyncStarted(unsigned long nowMillis) {
  if (_lastGPSSyncTime > 0) {
    return ((nowMillis - _lastGPSSyncTime)/1000 < GPS_READING_VALIDITY_SECS);
  } else {
    return false;
  }
}

// ************************************************************
// The last time we last got a GPS update
// ************************************************************
unsigned long GPSManager_::getLastGPSReadTime() {
  return _lastGPSReadTime;
}

// ************************************************************
// Get the last GPS time we read
// ************************************************************
time_t GPSManager_::getLastGPSTime() {
  return _lastGPSTime;
}

// ************************************************************
// Get the last raw GPS time string we read
// ************************************************************
String GPSManager_::getLastGPSTimeRaw() {
  return _lastGPSTimeRaw;
}

// ************************************************************
// Get the singleton instance
// ************************************************************
GPSManager_ &GPSManager_::getInstance() {
  static GPSManager_ instance;
  return instance;
}

GPSManager_ &gpsManager = gpsManager.getInstance();
