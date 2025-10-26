#pragma once

#include <memory>
#include <Arduino.h>
#include <AsyncUDP.h>
#include <WiFi.h>
#include <DNSServer.h>          //https://github.com/esp8266/Arduino/tree/master/libraries/DNSServer
#include "Defs.h"               // for DEBUG setting
#include "DebugManager.h"

typedef enum quote_direction {
  none = 'X',
  up = 'U',
  down = 'D',
  unchanged = '-'
} quote_direction_t;

#include "Globals.h"

// ------------------------ Types ------------------------

typedef void (*NewQuoteCallback) ();

// ------------------------ NTP ------------------------

#define QTE_HOST_DEFAULT "tzs.nixieclock.biz"
#define QTE_PORT_DEFAULT 2222
#define QTE_REQUEST_PACKET_SIZE 8
#define QTE_RESPONSE_PACKET_SIZE 16

class QuoteManager_
{
  private:
    QuoteManager_() = default; // Make constructor private

  public:
    static QuoteManager_ &getInstance(); // Accessor for singleton instance

    QuoteManager_(const QuoteManager_ &) = delete; // no copying
    QuoteManager_ &operator=(const QuoteManager_ &) = delete;

  public:
    void setDebugOutput(bool newDebug);
    void getQuote();
    bool getIsQuoteValid();
    int  getLastQuote();
    quote_direction_t getLastQuoteDirection();
    
    // callbacks
    void setNewQuoteCallback(NewQuoteCallback nqcb);
  private:
    String   _serverAddr = QTE_HOST_DEFAULT;              // The UDP quote server we are using
    int      _serverPort = QTE_PORT_DEFAULT;              // The UDP port on the quote server we are using
    unsigned long _qteStarted = 0;                        // the millis the request was sent at
    unsigned long _lastUpdateFromServer = 0;              // The last millis() we got an update at
    bool _quoteValid = false;                             // Tells us if the quote has been retrieved
    int _quoteValue = 0;                                  // The value of the last quote
    quote_direction_t _quoteDirection = unchanged;          // The value of the last quote is below MA

    AsyncUDP _udp;
    NewQuoteCallback _nqcb;
};

extern QuoteManager_ &quoteManager;