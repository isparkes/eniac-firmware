#pragma once

#include <Arduino.h>
#include "Defs.h"
#include "Globals.h"
#include "utilities.h"
#include "OLED.h"
#include "wps.h"
#include <ESPmDNS.h>
#include "WebManager.h"
#ifdef FEATURE_MENU
#include "MenuManager.h"
#endif
#include <DNSServer.h>

const byte    DNS_PORT                = 53;

class WiFiManager_ {
  private:
    WiFiManager_() = default; // Make constructor private

  public:
    static WiFiManager_ &getInstance(); // Accessor for singleton instance

    WiFiManager_(const WiFiManager_ &) = delete; // no copying
    WiFiManager_ &operator=(const WiFiManager_ &) = delete;

  public:
    void setUpWiFi();

    void connectToLastAP();
    bool connectWithWPS();
    void openAccessPortal();
    void startSmartConfig();

    void startWiFiServices();
    void startWiFiServicesPortal();
    void resetWiFiCredentials();
    bool wifiCredentialsReceived();
    void disconnectWiFi();

    void startScanWiFiNetworks();
    int getLastScanResultCount();
    String getLastScanResultSSID(int index);
    void wifiBeginWithCredentials();
    void saveWiFiCredentials(String newWiFiSSID, String newWiFiPassword);
    void processScanResults();

    // For captive portal
    void startDNSD();
    void stopDNSD();
    void manageDNSInOpenAP();

    // WiFi events are handled on the arduino_events task, which must
    // not block and must not touch shared state. The event handler
    // just sets these, and serviceEvents() does the work in loop()
    volatile bool pendingAPStart = false;
    volatile bool pendingGotIP = false;
    volatile bool pendingWPSSuccess = false;
    volatile bool pendingScanDone = false;
    void serviceEvents();

  private:
    bool _isOpenAP = false;
    std::unique_ptr<DNSServer>        dnsServer;    

    // For resolving names to esp32xxxxx.local
    void startMDNS();
};

extern WiFiManager_ &wifiManager;