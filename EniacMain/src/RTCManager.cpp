#include "RTCManager.h"

// ************************************************************
// Convert normal decimal to binary coded decimal numbers
// ************************************************************
uint8_t RtcManager_::decToBcd(uint8_t val)
{
    return ( (val/10*16) + (val%10) );
}

// ************************************************************
// Convert binary coded decimal to normal decimal numbers
// ************************************************************
uint8_t RtcManager_::bcdToDec(uint8_t val)
{
    return ( (val/16*10) + (val%16) );
}

// only need to call if you did not already start Wire
void RtcManager_::begin()
{
    Wire.begin();
}

// ************************************************************
// Start the hardware RTC
// ************************************************************
void RtcManager_::startClock(void)                    // set the ClockHalt bit low to start the rtc
{
  Wire.beginTransmission(RtcManager_I2C_ADDRESS);
  Wire.write(0x00);                      // Register 0x00 holds the oscillator start/stop bit
  Wire.endTransmission();
  Wire.requestFrom(RtcManager_I2C_ADDRESS, 1);
  _second = Wire.read() & 0x7f;                    // save actual seconds and AND sec with bit 7 (sart/stop bit) = clock started
  Wire.beginTransmission(RtcManager_I2C_ADDRESS);
  Wire.write(0x00);
  Wire.write(_second);                    // write seconds back and start the clock
  Wire.endTransmission();
}

// ************************************************************
// Stop the hardware RTC
// ************************************************************
void RtcManager_::stopClock(void)                     // set the ClockHalt bit high to stop the rtc
{
  Wire.beginTransmission(RtcManager_I2C_ADDRESS);
  Wire.write(0x00);                      // Register 0x00 holds the oscillator start/stop bit
  Wire.endTransmission();
  Wire.requestFrom(RtcManager_I2C_ADDRESS, 1);
  _second = Wire.read() | 0x80;                    // save actual seconds and OR sec with bit 7 (sart/stop bit) = clock stopped
  Wire.beginTransmission(RtcManager_I2C_ADDRESS);
  Wire.write(0x00);
  Wire.write(_second);                    // write seconds back and stop the clock
  Wire.endTransmission();
}

// ************************************************************
// Get the display time back from the hardware
// ************************************************************
void RtcManager_::getTimeRTCHardware()
{
  // Reset the register pointer
  Wire.beginTransmission(RtcManager_I2C_ADDRESS);
  Wire.write((uint8_t)0x00);
  Wire.endTransmission();  
  Wire.requestFrom(RtcManager_I2C_ADDRESS, 7);
  // A few of these need masks because certain bits are control bits
  _second     = bcdToDec(Wire.read() & 0x7f);
  _minute     = bcdToDec(Wire.read());
  _hour       = bcdToDec(Wire.read() & 0x3f);    // Need to change this if 12 hour am/pm
  _dayOfWeek  = bcdToDec(Wire.read());
  _dayOfMonth = bcdToDec(Wire.read());
  _month      = bcdToDec(Wire.read());
  _year       = bcdToDec(Wire.read());

  #ifdef RTC_EXTENDED_DEBUG
  debugMsg("Got RTC " + String(_year) + ", " + String(_month) + ", " + String(_dayOfMonth) + ", " + String(_hour) + ", " + String(_minute) + ", " + String(_second));
  #endif
}

// ************************************************************
// Put the display time into the RTC hardware
// ************************************************************
void RtcManager_::setTimeRTCHardware()
{
    Wire.beginTransmission(RtcManager_I2C_ADDRESS);
    Wire.write((uint8_t)0x00);
    Wire.write(decToBcd(_second));// 0 to bit 7 starts the clock
    Wire.write(decToBcd(_minute));
    Wire.write(decToBcd(_hour));                   // If you want 12 hour am/pm you need to set bit 6 
    Wire.write(decToBcd(_dayOfWeek));
    Wire.write(decToBcd(_dayOfMonth));
    Wire.write(decToBcd(_month));
    Wire.write(decToBcd(_year));
    Wire.endTransmission();

    debugMsgRtc("Set hardware RTC " + String(_year) + ", " + String(_month) + ", " + String(_dayOfMonth) + ", " + String(_hour) + ", " + String(_minute) + ", " + String(_second));
}

// ************************************************************
// Check that the RTC is actually running
// ************************************************************
unsigned char RtcManager_::isRunning()
{
  Wire.beginTransmission(RtcManager_I2C_ADDRESS);
  Wire.write((uint8_t)0x00); 
  Wire.endTransmission();

  // Just fetch the seconds register and check the top bit
  Wire.requestFrom(RtcManager_I2C_ADDRESS, 1);
  return !(Wire.read() & 0x80);
}

// ************************************************************
// Check that we still have access to the time from the RTC
// ************************************************************
bool RtcManager_::testRTCTimeProvider() {
  debugMsgRtc("Testing RTC");
  Wire.beginTransmission(RtcManager_I2C_ADDRESS);
  _useRTC = (Wire.endTransmission(true) == 0);
  debugMsgRtc("Set useRTC to: " + String(_useRTC));
  if (!_useRTC) {
    debugMsgRtc("I2C error: " + String(Wire.getErrorText(Wire.lastError())));
  }

  if (!_onceHadAnRTC && _useRTC) {
    _onceHadAnRTC = true;
  }
  return _useRTC;
}

// ************************************************************
// Get the time from the RTC, returns as UTC
// ************************************************************
time_t RtcManager_::getRTCTimeAsTimeT() {
  if (_useRTC) {
    getTimeRTCHardware();

  // We store the date 0-99 in the RTC, but mktime gives us back the years since 1900
  // mktime gives us months in the range 0-11, but the RTC needs 1-12
    struct tm rtcCurrentTm;
    rtcCurrentTm.tm_year = _year + 100;
    rtcCurrentTm.tm_mon = _month - 1; 
    rtcCurrentTm.tm_mday = _dayOfMonth; 
    rtcCurrentTm.tm_hour = _hour; 
    rtcCurrentTm.tm_min = _minute;
    rtcCurrentTm.tm_sec = _second;
    rtcCurrentTm.tm_isdst = tzManager.getCurrentUTCIsDST();

    #ifdef RTC_EXTENDED_DEBUG
    debugMsg("---->Applying offset: " + String(tzManager.getCurrentUTCOffset()) + " to RTC harware time -> " + 
      String(rtcCurrentTm.tm_year) + ", " + String(rtcCurrentTm.tm_mon) + ", " + String(rtcCurrentTm.tm_mday) + ", " + 
      String(rtcCurrentTm.tm_hour) + ", " + String(rtcCurrentTm.tm_min) + ", " + String(rtcCurrentTm.tm_sec));
    #endif

    // mktime gives us the opposite of what local time does, so we have to adjust it by adding in the offset mktime will apply
    time_t _rtctime = mktime(&rtcCurrentTm) + tzManager.getCurrentUTCOffset();

    debugMsgRtc("Recovered RTC time from hardware: " + String(_rtctime) + " U--> " + tzManager.gmtimeToReadableString(_rtctime));

    return _rtctime;
  } else {
    return 0;
  }
}

// ************************************************************
// Set the time from the value we get back from a UTC time source
// ************************************************************
void RtcManager_::setTimeFromUTCSource(time_t currentTime, bool updateRTC) {

  struct tm info_gm;
  gmtime_r(&currentTime, &info_gm);

  // We store the date 0-99 in the RTC, but gmtime gives us back the years since 1900
  // gmtime gives us months in the range 0-11, but the RTC needs 1-12
  _year = info_gm.tm_year % 100;
  _month = info_gm.tm_mon + 1;
  _dayOfMonth = info_gm.tm_mday;
  _hour = info_gm.tm_hour;
  _minute = info_gm.tm_min;
  _second = info_gm.tm_sec;

  if (updateRTC) {
    // Push the update to the RTC chip
    setTimeRTCHardware();
  }
    
  debugMsgRtc("Set RTC time to time: " + String(currentTime) + " U--> " + tzManager.gmtimeToReadableString(currentTime));
}

// ************************************************************
// Was the RTC valid the last time we checked
// ************************************************************
bool RtcManager_::getRTCValid() {
  return _useRTC;
}

// ************************************************************
// Get singleton instance
// ************************************************************
RtcManager_ &RtcManager_::getInstance() {
  static RtcManager_ instance;
  return instance;
}

RtcManager_ &rtcManager = rtcManager.getInstance();
