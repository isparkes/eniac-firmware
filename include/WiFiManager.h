#pragma once

#include <Arduino.h>
#include "defs.h"
#include "globals.h"
#include "utilities.h"
#include "OLED.h"
#include "wps.h"
#include <ESPmDNS.h>
#include "WebManager.h"
#include "MenuManager.h"

void setUpWiFi();

void connectToLastAP();
bool connectWithWPS();
void openAccessPortal();
void startSmartConfig();

void startWiFiServices();
void startWiFiServicesPortal();
void startMDNS();
void resetWiFiCredentials();
bool wifiCredentialsReceived();
void disconnectWiFi();

void startScanWiFiNetworks();
