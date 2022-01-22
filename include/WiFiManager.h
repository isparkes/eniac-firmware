#pragma once

#include <Arduino.h>
#include "defs.h"
#include "globals.h"
#include "utilities.h"
#include "OLED.h"
#include "wps.h"
#include <ESPmDNS.h>

void setUpWiFi();
bool connectToLastAP();
void startMDNS();
void ScanWiFiNetworks();
void openAccessPortal();
