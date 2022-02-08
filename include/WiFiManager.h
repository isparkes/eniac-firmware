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
void startWiFiServices();
void startMDNS();
void disconnectWiFi();
void resetWiFiCredentials();
bool wifiCredentialsReceived();

bool connectWithWPS();
void scanWiFiNetworks();
void openAccessPortal();
