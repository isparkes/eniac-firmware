#include "ESP_DS1307.h"

uint8_t DS1307_::decToBcd(uint8_t val)
{
    return ( (val/10*16) + (val%10) );
}

//Convert binary coded decimal to normal decimal numbers
uint8_t DS1307_::bcdToDec(uint8_t val)
{
    return ( (val/16*10) + (val%16) );
}

// only need to call if you did not already start Wire
void DS1307_::begin()
{
    Wire.begin();
}

// ToDo; Perhaps we can remove all the casts - they seem to be redundant
void DS1307_::startClock(void)                    // set the ClockHalt bit low to start the rtc
{
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write((uint8_t)0x00);                      // Register 0x00 holds the oscillator start/stop bit
  Wire.endTransmission();
  Wire.requestFrom(DS1307_I2C_ADDRESS, 1);
  _second = Wire.read() & 0x7f;                    // save actual seconds and AND sec with bit 7 (sart/stop bit) = clock started
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write((uint8_t)0x00);
  Wire.write((uint8_t)_second);                    // write seconds back and start the clock
  Wire.endTransmission();
}

void DS1307_::stopClock(void)                     // set the ClockHalt bit high to stop the rtc
{
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write((uint8_t)0x00);                      // Register 0x00 holds the oscillator start/stop bit
  Wire.endTransmission();
  Wire.requestFrom(DS1307_I2C_ADDRESS, 1);
  _second = Wire.read() | 0x80;                    // save actual seconds and OR sec with bit 7 (sart/stop bit) = clock stopped
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write((uint8_t)0x00);
  Wire.write((uint8_t)_second);                    // write seconds back and stop the clock
  Wire.endTransmission();
}

void DS1307_::getTimeInternal()
{
    // Reset the register pointer
    Wire.beginTransmission(DS1307_I2C_ADDRESS);
    Wire.write((uint8_t)0x00);
    Wire.endTransmission();  
    Wire.requestFrom(DS1307_I2C_ADDRESS, 7);
    // A few of these need masks because certain bits are control bits
    _second     = bcdToDec(Wire.read() & 0x7f);
    _minute     = bcdToDec(Wire.read());
    _hour       = bcdToDec(Wire.read() & 0x3f);    // Need to change this if 12 hour am/pm
    _dayOfWeek  = bcdToDec(Wire.read());
    _dayOfMonth = bcdToDec(Wire.read());
    _month      = bcdToDec(Wire.read());
    _year       = bcdToDec(Wire.read());
}

void DS1307_::setTimeInternal()
{
    Wire.beginTransmission(DS1307_I2C_ADDRESS);
    Wire.write((uint8_t)0x00);
    Wire.write(decToBcd(_second));// 0 to bit 7 starts the clock
    Wire.write(decToBcd(_minute));
    Wire.write(decToBcd(_hour));                   // If you want 12 hour am/pm you need to set bit 6 
    Wire.write(decToBcd(_dayOfWeek));
    Wire.write(decToBcd(_dayOfMonth));
    Wire.write(decToBcd(_month));
    Wire.write(decToBcd(_year));
    Wire.endTransmission();
}

void DS1307_::fillByHMS(uint8_t hour, uint8_t minute, uint8_t second)
{
    // assign variables
    _hour = hour;
    _minute = minute;
    _second = second;
}

void DS1307_::fillByYMD(uint8_t year, uint8_t month, uint8_t day)
{
    _year = year;
    _month = month;
    _dayOfMonth = day;
}

void DS1307_::fillDayOfWeek(uint8_t dow)
{
    _dayOfWeek = dow;
}

// ************************************************************
// Check that the RTC is actually running
// ************************************************************
unsigned char DS1307_::isRunning()
{
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write((uint8_t)0x00); 
  Wire.endTransmission();

  // Just fetch the seconds register and check the top bit
  Wire.requestFrom(DS1307_I2C_ADDRESS, 1);
  return !(Wire.read() & 0x80);
}

// ************************************************************
// Check that we still have access to the time from the RTC
// ************************************************************
bool DS1307_::testRTCTimeProvider() {
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  _useRTC = (Wire.endTransmission(true) == 0);
  #ifdef DEBUG_ON
  debugMsg("Set useRTC to: " + String(_useRTC));
  if (!_useRTC) {
    debugMsg("I2C error: " + String(Wire.getErrorText(Wire.lastError())));
  }
  #endif
  return _useRTC;
}

// ************************************************************
// Get the time from the RTC
// ************************************************************
String DS1307_::getRTCTime(bool setInternalTime, unsigned long nowMillis) {
  if (_useRTC) {
    getTimeInternal();
    int years = _year + 2000;

    String returnValue = timeToReadableString(years, _month, _dayOfMonth, _hour, _minute, _second);

    _lastRTCSetTime = nowMillis;

    #ifdef DEBUG_ON
    debugMsg("Got RTC time: " + returnValue);
    #endif

    if (setInternalTime) {
      // Set the internal time provider to the value we got
      setTime(_hour, _minute, _second, _dayOfMonth, _month, years);
      #ifdef DEBUG_ON
      debugMsg("Set Internal time to: " + returnValue);
      #endif
    }

    return returnValue;
  } else {
    return "";
  }
}

// ************************************************************
// Get the time from the RTC, extrapolating from the last time
// We knew and adding the offset to it
// ************************************************************
String DS1307_::getEstimatedCurrentRTCTime(unsigned long nowMillis) {
  if (_useRTC) {
    struct tm lastSetTime;
    lastSetTime.tm_year = _year + 100;
    lastSetTime.tm_mon = _month - 1; 
    lastSetTime.tm_mday = _dayOfMonth; 
    lastSetTime.tm_hour = _hour; 
    lastSetTime.tm_min = _minute;
    lastSetTime.tm_sec = _second;

    time_t lastSetTimeT = mktime(&lastSetTime);

    int secondsSinceUpdate = (nowMillis - _lastRTCSetTime) / 1000;

    time_t estimatedCurrentTime = lastSetTimeT + secondsSinceUpdate;

    const tm* tm = localtime(&estimatedCurrentTime);

    String timeString = String(tm->tm_year + 1900) + "," + String(tm->tm_mon + 1) + "," + String(tm->tm_mday) + "," + String(tm->tm_hour) + "," + String(tm->tm_min) + "," + String(tm->tm_sec);
    #ifdef DEBUG_ON
    debugMsg("RTC time str: " + timeString);
    #endif

    return timeString;    
  } else {
    return "";
  }
}

// ************************************************************
// The last time we last got a RTC update
// ************************************************************
unsigned long DS1307_::getLastRTCSetTime() {
  return _lastRTCSetTime;
}

// ************************************************************
// Set the date/time in the RTC from the internal time
// Always hold the time in 24 format, we convert to 12 in the
// display.
// ************************************************************
void DS1307_::setRTCTime(unsigned long nowMillis) {
  if (_useRTC) {
    fillByYMD(year() % 100, month(), day());
    fillByHMS(hour(), minute(), second());
    setTimeInternal();

    _lastRTCSetTime = nowMillis;

    #ifdef DEBUG_ON
    debugMsg("Set RTC time to internal time: " + String(year()) + ":" + String(month()) + ":" + String(day()) + " " + String(hour()) + ":" + String(minute()) + ":" + String(second()));
    #endif
  }
}

// ************************************************************
// Set the time from the value we get back from the time server
// ************************************************************
void DS1307_::setTimeFromServer(String timeString, unsigned long nowMillis) {
  #define SYNC_HOURS 3
  #define SYNC_MINS 4
  #define SYNC_SECS 5
  #define SYNC_DAY 2
  #define SYNC_MONTH 1
  #define SYNC_YEAR 0

  int intValues[6];
  grabInts(timeString, &intValues[0], ",");
  setTime(intValues[SYNC_HOURS], intValues[SYNC_MINS], intValues[SYNC_SECS], intValues[SYNC_DAY], intValues[SYNC_MONTH], intValues[SYNC_YEAR]);

  // Push the update to the RTC chip
  setRTCTime(nowMillis);
  
  #ifdef DEBUG_ON
  debugMsg("Set RTC time to NTP time: " + String(year()) + ":" + String(month()) + ":" + String(day()) + " " + String(hour()) + ":" + String(minute()) + ":" + String(second()));
  #endif
}

// ************************************************************
// Was the RTC valid the last time we checked
// ************************************************************
bool DS1307_::getRTCValid() {
  return _useRTC;
}


// ************************************************************
// Get singleton instance
// ************************************************************
DS1307_ &DS1307_::getInstance() {
  static DS1307_ instance;
  return instance;
}

DS1307_ &rtclock = rtclock.getInstance();
