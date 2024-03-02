#include "NTPManager.h"

// ************************************************************
// reset all the internal defaults
// ************************************************************
void NtpManager_::resetDefaults() {
  setNtpPool(NTP_POOL_DEFAULT);
  setUpdateInterval(NTP_UPDATE_INTERVAL_DEFAULT);
  debugMsgNtp("Reset defaults");
}

// ************************************************************
// set the update interval
// ************************************************************
void NtpManager_::setUpdateInterval(int updateInterval) {
  _ntpUpdateInterval = updateInterval;
}

// ************************************************************
// get the update interval
// ************************************************************
int NtpManager_::getUpdateInterval() {
  return _ntpUpdateInterval;
}

// ************************************************************
// set the NTP pool
// ************************************************************
void NtpManager_::setNtpPool(String ntpPool) {
  _ntpPool = ntpPool;
}

// ************************************************************
// get the NTP pool
// ************************************************************
String NtpManager_::getNtpPool() {
  return _ntpPool;
}

// ************************************************************
// get the number of millis until the next update is due in seconds
// ************************************************************
signed int NtpManager_::getNextUpdate() {
  // deal with the startup case - we always want to update 
  if (_lastUpdateFromServer == 0) {
    return -1;
  } else {
    return (_lastUpdateFromServer/1000 + _ntpUpdateInterval) - nowMillis/1000;
  }
}

// ************************************************************
// get the update interval
// ************************************************************
unsigned long NtpManager_::getLastUpdate() {
  return _lastUpdateFromServer;
}

// ************************************************************
// Invalidate the last update time and trigger a new NTP update
// ************************************************************
void NtpManager_::resetNextUpdate() {
  _lastUpdateFromServer = 0;
}

// ************************************************************
// get the last time we got back
// ************************************************************
time_t NtpManager_::getLastTimeTFromServer() {
  return _ntpTime;
}

// ************************************************************
// see if the NTP we got is still to be condsidered valid
// ************************************************************
bool NtpManager_::ntpTimeValid() {
  return _lastUpdateFromServer != 0 && ((nowMillis - _lastUpdateFromServer) < (2000 * _ntpUpdateInterval));
}

// ************************************************************
// Asynchronous NTP query
// ************************************************************
void NtpManager_::getTimeFromNTP() {
  debugMsgNtp("Async NTP in");

  if (WiFi.status() != WL_CONNECTED) {
    debugMsgNtp("WiFi not connected. Abort.");
    return;
  }


  // Don't spam the pool
  if ((_ntpStarted > 0) && (nowMillis - _ntpStarted) < NTP_RETRY_INTERVAL_MS) {
    debugMsgNtp("Suppressing NTP retry");
    return;
  }
  
  IPAddress serverIP;
  WiFi.hostByName(_ntpPool.c_str(), serverIP);
  #ifdef DEBUG
  String str = "NTP IPAddr: ";
  str += serverIP.toString();
  debugMsgNtp(str);
  #endif

  byte buffer[NTP_PACKET_SIZE];
  memset(buffer, 0, NTP_PACKET_SIZE);
  buffer[0] = 0b11100011;   // LI, Version, Mode
  buffer[1] = 0;            // Stratum, or type of clock
  buffer[2] = 9;            // Polling Interval (9 = 2^9 secs = ~9 mins, close to our 10 min default)
  buffer[3] = 0xEC;         // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  buffer[12]  = 'X';        // "kiss code", see RFC5905
  buffer[13]  = 'W';        // (codes starting with 'X' are not interpreted)
  buffer[14]  = 'N';
  buffer[15]  = 'C';

  _ntpStarted = nowMillis;
  #ifdef NTP_EXTENDED_DEBUG
  debugMsgNtp("Connect to NTP");
  #endif
  _udp.connect(serverIP, 123); //NTP requests are to port 123
  #ifdef NTP_EXTENDED_DEBUG
  debugMsgNtp("Write to NTP");
  #endif
  _udp.write(buffer, NTP_PACKET_SIZE);
  #ifdef NTP_EXTENDED_DEBUG
  debugMsgNtp("NTP Packet sent at " + String(_ntpStarted));
  #endif

  _udp.onPacket([&](AsyncUDPPacket packet) {
    #ifdef NTP_EXTENDED_DEBUG
    String message = "NTP response UDP Packet Type: " + packet.isBroadcast() ? "Broadcast" : packet.isMulticast() ? "Multicast" : "Unicast";
    message += " from " + packet.remoteIP().toString() + ":" + String(packet.remotePort());
    message += " to " + packet.localIP().toString() + ":" + String(packet.localPort());
    debugMsgNtp(message);
    #endif

    if (packet.length() != 48) {
      debugMsgNtp("Received data, but got invalid length: " + String(packet.length()));
    } else {
      uint8_t buffer[NTP_PACKET_SIZE];
      memcpy(&buffer, packet.data(), packet.length());
      //prepare timestamps
      uint32_t highWord, lowWord;
      highWord = ( buffer[16] << 8 | buffer[17] ) & 0x0000FFFF;
      lowWord = ( buffer[18] << 8 | buffer[19] ) & 0x0000FFFF;
      uint32_t reftsSec = highWord << 16 | lowWord;       // reference timestamp seconds

      highWord = ( buffer[32] << 8 | buffer[33] ) & 0x0000FFFF;
      lowWord = ( buffer[34] << 8 | buffer[35] ) & 0x0000FFFF;
      uint32_t rcvtsSec = highWord << 16 | lowWord;       // receive timestamp seconds

      highWord = ( buffer[40] << 8 | buffer[41] ) & 0x0000FFFF;
      lowWord = ( buffer[42] << 8 | buffer[43] ) & 0x0000FFFF;
      uint32_t secsSince1900 = highWord << 16 | lowWord;      // transmit timestamp seconds

      highWord = ( buffer[44] << 8 | buffer[45] ) & 0x0000FFFF;
      lowWord = ( buffer[46] << 8 | buffer[47] ) & 0x0000FFFF;
      #ifdef NTP_EXTENDED_DEBUG
      uint32_t fraction = highWord << 16 | lowWord;       // transmit timestamp fractions
      #endif

      //check if received data makes sense
      //buffer[1] = stratum - should be 1..15 for valid reply
      //also checking that all timestamps are non-zero and receive timestamp seconds are <= transmit timestamp seconds
      if ((buffer[1] < 1) or (buffer[1] > 15) or (reftsSec == 0) or (rcvtsSec == 0) or (rcvtsSec > secsSince1900)) {
        // we got invalid packet
        debugMsgNtp("Got data, but got INVALID_DATA");
        return;
      }

      // Set the t and measured_at variables that were passed by reference
      unsigned long done = millis();
      unsigned long t = secsSince1900 - 2208988800UL;                     // Subtract 70 years to get seconds since 1970
      #ifdef NTP_EXTENDED_DEBUG
      debugMsgNtp("Whole seconds " + String(t));
      #ifdef DEBUG
      uint16_t ms = fraction / 4294967UL;
      debugMsgNtp("Fractional " + String(ms));
      #endif
      debugMsgNtp("success round trip " + String(done - _ntpStarted) + " ms");
      #endif

      _lastUpdateFromServer = done;

      #ifdef DEBUG
      unsigned long measured_at = (done - _ntpStarted) / 2;  // Assume symmetric network latency and return when we think the whole second was.
      debugMsgNtp("lastUpdateFromServer: @" + String(_lastUpdateFromServer) + " - latency: " + String(measured_at));
      #endif
      
      // Get the current time in mS, assuming symmtric latency
      _ntpTime = t;

      debugMsgNtp("Raw time: " + String(_ntpTime));

      // Reset when we last started.
      _ntpStarted = 0;

      // Notify the outside world that we have updated
      if (_ntcb != NULL) {
        _ntcb();
      }
    }
  });

  debugMsgNtp("Async GET Time out");
}

// ************************************************************
// Set the callback for informing that a new time update is there
// ************************************************************
void NtpManager_::setNewTimeCallback(NewTimeCallback ntcb) {
  _ntcb = ntcb;
  debugMsgNtp("Time update callback set");
}

NtpManager_ &NtpManager_::getInstance() {
  static NtpManager_ instance;
  return instance;
}

NtpManager_ &ntpManager = ntpManager.getInstance();