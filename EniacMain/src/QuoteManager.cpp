#include "QuoteManager.h"

// ************************************************************
// Asynchronous NTP query
// ************************************************************
void QuoteManager_::getQuote() {
  debugMsgQte("Async QTE in");

  if (WiFi.status() != WL_CONNECTED) {
    debugMsgQte("WiFi not connected. Abort.");
    return;
  }

  _quoteValid = false;

  IPAddress serverIP;
  WiFi.hostByName(_serverAddr.c_str(), serverIP);
  #ifdef DEBUG
  String str = "NTP IPAddr: ";
  str += serverIP.toString();
  debugMsgQte(str);
  #endif

  byte buffer[QTE_REQUEST_PACKET_SIZE];
  memset(buffer, ' ', QTE_REQUEST_PACKET_SIZE);
  // The first character tells us the formatting we wamt, in this case as an integer value
  buffer[0] = 'I';

  // The rest of the buffer tells us the ticker
  buffer[1] = 'B';
  buffer[2] = 'T';
  buffer[3] = 'C';
  buffer[4] = 'U';
  buffer[5] = 'S';
  buffer[6] = 'D';
  buffer[7] = 'T';

  _qteStarted = nowMillis;
  #ifdef QTE_EXTENDED_DEBUG
  debugMsgQte("Connect to UDP quote server");
  #endif
  _udp.connect(serverIP, QTE_PORT_DEFAULT);
  #ifdef QTE_EXTENDED_DEBUG
  debugMsgQte("Write to QTE server");
  #endif
  _udp.write(buffer, QTE_REQUEST_PACKET_SIZE);
  #ifdef QTE_EXTENDED_DEBUG
  debugMsgQte("QTE Packet sent at " + String(_qteStarted));
  #endif

  _udp.onPacket([&](AsyncUDPPacket packet) {
    #ifdef QTE_EXTENDED_DEBUG
    String message = "QTE response UDP Packet Type: " + packet.isBroadcast() ? "Broadcast" : packet.isMulticast() ? "Multicast" : "Unicast";
    message += " from " + packet.remoteIP().toString() + ":" + String(packet.remotePort());
    message += " to " + packet.localIP().toString() + ":" + String(packet.localPort());
    debugMsgQte(message);
    #endif

    if (packet.length() != QTE_RESPONSE_PACKET_SIZE) {
      debugMsgQte("Received data, but got invalid length: " + String(packet.length()));
    } else {
      uint8_t buffer[QTE_RESPONSE_PACKET_SIZE];
      memset(buffer, 0, QTE_RESPONSE_PACKET_SIZE);
      memcpy(&buffer, packet.data(), packet.length());

      _lastUpdateFromServer = nowMillis;

      String quoteStr = String((char *)buffer);

      debugMsgQte("Quote: " + quoteStr);
      _quoteValue = quoteStr.toInt();

      // Reset when we last started.
      _qteStarted = 0;
      _quoteValid = true;

      // Notify the outside world that we have updated
      if (_nqcb != NULL) {
        _nqcb();
      }
    }
  });

  debugMsgQte("Async QTE out");
}

// ************************************************************
// see if the NTP we got is still to be condsidered valid
// ************************************************************
bool QuoteManager_::getIsQuoteValid() {
  return _quoteValid;
}

// ************************************************************
// see if the NTP we got is still to be condsidered valid
// ************************************************************
int QuoteManager_::getLastQuote() {
  return _quoteValue;
}

// ************************************************************
// Set the callback for informing that a new time update is there
// ************************************************************
void QuoteManager_::setNewQuoteCallback(NewTimeCallback nqcb) {
  _nqcb = nqcb;
  debugMsgQte("Quote update callback set");
}

QuoteManager_ &QuoteManager_::getInstance() {
  static QuoteManager_ instance;
  return instance;
}

QuoteManager_ &quoteManager = quoteManager.getInstance();