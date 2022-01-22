#pragma once

#include <memory>
#include <ESPAsyncWebServer.h>
#include "globals.h"
#include "utilities.h"
#include "DebugManager.h"

class WebManager_ {
  private:
    WebManager_() = default; // Make constructor private

  public:
    static WebManager_ &getInstance(); // Accessor for singleton instance

    WebManager_(const WebManager_ &) = delete; // no copying
    WebManager_ &operator=(const WebManager_ &) = delete;
    
    // Turn off or on logging
    void setDebugOutput(bool newDebug);
    
    // callbacks
    void setDebugCallback(DebugCallback dbcb);
  public:
    void begin();
    void beginWiFiCredentials();
    void startOTA();
  private:
    DebugCallback _dbcb;
    bool _debug = false;

    void debugMsg(String message);      
};

extern AsyncWebServer server;

extern WebManager_ &webManager;