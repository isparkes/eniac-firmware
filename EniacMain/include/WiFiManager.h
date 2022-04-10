#pragma once

#include <Arduino.h>
#include "Defs.h"
#include "Globals.h"
#include "utilities.h"
#include "OLED.h"
#include "wps.h"
#include <ESPmDNS.h>
#include "WebManager.h"
#include "MenuManager.h"
#include "StringArray.h"

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
  private:
    StringArray ssidList;

    void startMDNS();
};

extern WiFiManager_ &wifiManager;