#include "MenuManager.h"

// -------------------------------------------------------------------------------------------------
//                                         menus below here
// -------------------------------------------------------------------------------------------------

void MenuManager_::mainMenu() {
  resetMenu();
  byte menuCount = 1;
  menuMode = menu;
  oledMenu.menuTitle = "Main Menu";
  oledMenu.menuItems[menuCount] = "Wifi";            oledMenu.menuActions[menuCount++] = gotoWifiMenu;
  oledMenu.menuItems[menuCount] = "Nixie Clock";     oledMenu.menuActions[menuCount++] = gotoDisplayMenu;
  oledMenu.menuItems[menuCount] = "System";          oledMenu.menuActions[menuCount++] = gotoOptionsMenu;
  if (tzManager.getPrimaryTimeSource() == TIME_SOURCE_RTC) {
    // only set the time via encoder when we are running from RTC
    oledMenu.menuItems[menuCount] = "Manual time set"; oledMenu.menuActions[menuCount++] = gotoTimeSetMenu;
  }
  oledMenu.menuItems[menuCount] = "Menu Off";        oledMenu.menuActions[menuCount++] = menuOff;
  oledMenu.noOfmenuItems = --menuCount;
}

void MenuManager_::wifiMenu() {
  resetMenu();
  menuMode = menu;
  String onOffMsg;
  byte menuCount = 1;
  String wcTag = "("+String(wifiManager.getLastScanResultCount())+")";
  if (WiFi.isConnected()) {
    oledMenu.menuTitle = "WiFi Menu";           
    oledMenu.menuItems[menuCount] = "Disconnect WiFi";         oledMenu.menuActions[menuCount++] = disconnectWifi;
    oledMenu.menuItems[menuCount] = "Back";                    oledMenu.menuActions[menuCount++] = backToMain;
  } else {
    oledMenu.menuTitle = "WiFi Menu";
    if (wifiManager.wifiCredentialsReceived()) {
      oledMenu.menuItems[menuCount] = "Reconnect previous";    oledMenu.menuActions[menuCount++] = reconnectPrevious;
    }
    oledMenu.menuItems[menuCount] = "Connect with WPS";        oledMenu.menuActions[menuCount++] = connectWPS;
    oledMenu.menuItems[menuCount] = "Start SmartConfig";       oledMenu.menuActions[menuCount++] = smartConfig;
    oledMenu.menuItems[menuCount] = "Open Access Point";       oledMenu.menuActions[menuCount++] = openAccessPoint;
    oledMenu.menuItems[menuCount] = "Scan Wifi";               oledMenu.menuActions[menuCount++] = scanWiFi;
    oledMenu.menuItems[menuCount] = "Select WiFi " + wcTag;    oledMenu.menuActions[menuCount++] = showWifiSelection;
    oledMenu.menuItems[menuCount] = "Enter SSID";              oledMenu.menuActions[menuCount++] = enterWiFiSSID;
    oledMenu.menuItems[menuCount] = "Enter password";          oledMenu.menuActions[menuCount++] = enterWiFiPassword;
    oledMenu.menuItems[menuCount] = "Back";                    oledMenu.menuActions[menuCount++] = backToMain;
  }
  oledMenu.noOfmenuItems = --menuCount;
}

void MenuManager_::nixieClockMenu() {
  resetMenu();
  menuMode = menu;
  byte menuCount = 1;
  oledMenu.menuTitle = "Nixie Clock";
  String status = cc->useLDRTube ? "off" : "on";
  oledMenu.menuItems[menuCount] = "Tube Dimming " + status;      oledMenu.menuActions[menuCount++] = toggleTubeDimming;
  status =  cc->useBLDim ? "off" : "on";
  oledMenu.menuItems[menuCount] = "BL Dimming " + status;        oledMenu.menuActions[menuCount++] = toggleBLDimming;
  if (cc->hourMode) {
    oledMenu.menuItems[menuCount] = "Set 24h mode";              oledMenu.menuActions[menuCount++] = toggleHourMode;
  } else {
    oledMenu.menuItems[menuCount] = "Set 12h mode";              oledMenu.menuActions[menuCount++] = toggleHourMode;
  }
  status = cc->fade ? "off" : "on";
  oledMenu.menuItems[menuCount] = "Digit Fade " + status;        oledMenu.menuActions[menuCount++] = toggleFade;
  status = cc->scrollback ? "off" : "on";
  oledMenu.menuItems[menuCount] = "Scrollback " + status;        oledMenu.menuActions[menuCount++] = toggleScrollback;
  oledMenu.menuItems[menuCount] = "Set Dimming value";           oledMenu.menuActions[menuCount++] = setDimming;
  #ifdef FEATURE_BLINKENLIGHTS
  String nextBLModeName = blinkenlightsManager.getNextBlinkenlightsModeName();
  oledMenu.menuItems[menuCount] = "IND mode: " + nextBLModeName; oledMenu.menuActions[menuCount++] = nextBlnknMode;
  #endif
  #ifdef NIXIE_SLAVE
  String nextSlaveModeName = slaveManagerNixie.getNextSlaveModeName();
  oledMenu.menuItems[menuCount] = "Slave Mode " + nextSlaveModeName; oledMenu.menuActions[menuCount++] = nextSlaveMode;
  #endif

  String nextACPMode = outputManager.getNextACPModeName();
  oledMenu.menuItems[menuCount] = "ACP: " + nextACPMode;         oledMenu.menuActions[menuCount++] = setNextACPMode;

  String nextSlotsMode = outputManager.getNextSlotsModeName();
  oledMenu.menuItems[menuCount] = "Date: " + nextSlotsMode;      oledMenu.menuActions[menuCount++] = setNextSlotsMode;

  String nextBlankMode = blankingManager.getNextBlankingModeName();
  oledMenu.menuItems[menuCount] = "Blank: " + nextBlankMode;     oledMenu.menuActions[menuCount++] = setNextBlankingMode;

  if(blankingManager.getCurrentModeWantsHours()) {
    oledMenu.menuItems[menuCount] = "Blank start hour";          oledMenu.menuActions[menuCount++] = setBlankingHourStart;
    oledMenu.menuItems[menuCount] = "Blank end hour";            oledMenu.menuActions[menuCount++] = setBlankingHourEnd;
  }

  oledMenu.menuItems[menuCount] = "Back";                        oledMenu.menuActions[menuCount++] = backToMain;
  oledMenu.noOfmenuItems = --menuCount;
}

void MenuManager_::systemMenu() {
  resetMenu();
  menuMode = menu;
  byte menuCount = 1;
  oledMenu.menuTitle = "System";
  oledMenu.menuItems[menuCount] = "Restart Device"; oledMenu.menuActions[menuCount++] = restartClock;
  oledMenu.menuItems[menuCount] = "Save config";    oledMenu.menuActions[menuCount++] = saveConfig;
  oledMenu.menuItems[menuCount] = "Save stats";     oledMenu.menuActions[menuCount++] = saveStats;
  #ifdef DIGIT_DIAGNOSTICS
  oledMenu.menuItems[menuCount] = "Display Test";   oledMenu.menuActions[menuCount++] = displayTest;
  #endif
  String status = cc->WifiOnAtStart ? "off" : "on";
  oledMenu.menuItems[menuCount] = "WiFi at start: "+ status; oledMenu.menuActions[menuCount++] = toggleWiFiAtStart;
  #ifdef DEBUG
  oledMenu.menuItems[menuCount] = "Debug on 10m";   oledMenu.menuActions[menuCount++] = debugOn10mins;
  #endif
  oledMenu.menuItems[menuCount] = "Set Location";   oledMenu.menuActions[menuCount++] = selectLocationArea;
  oledMenu.menuItems[menuCount] = "Reset WiFi";     oledMenu.menuActions[menuCount++] = resetWiFiInfo;
  oledMenu.menuItems[menuCount] = "Back";           oledMenu.menuActions[menuCount++] = backToMain;
  oledMenu.noOfmenuItems = --menuCount;
}

void MenuManager_::setTimeMenu() {
  resetMenu();
  menuMode = menu;
  byte menuCount = 1;
  oledMenu.menuTitle = "Set Time";
  oledMenu.menuItems[menuCount] = "Set Hours";      oledMenu.menuActions[menuCount++] = setHours;
  oledMenu.menuItems[menuCount] = "Set minutes";    oledMenu.menuActions[menuCount++] = setMinutes;
  oledMenu.menuItems[menuCount] = "Back";           oledMenu.menuActions[menuCount++] = backToMain;
  oledMenu.noOfmenuItems = --menuCount;
}

void MenuManager_::wifiSelectMenu() {
  resetMenu();
  menuMode = menu;
  byte menuCount = 1;
  oledMenu.menuTitle = "Select network";

  int scanCount = wifiManager.getLastScanResultCount();
  int numberOfEntries = scanCount < maxmenuItems ? scanCount : maxmenuItems;

  // Leave some room for the "Back" option, but ensure we don't go negative
  if (numberOfEntries > 0) {
    numberOfEntries--;
  }
  debugMsgMnm("Showing entries: " + String(numberOfEntries));

  if (scanCount < numberOfEntries) numberOfEntries = scanCount;
  for (int i = 0; i < numberOfEntries ; i++) {
    oledMenu.menuItems[menuCount] = wifiManager.getLastScanResultSSID(i); oledMenu.menuActions[menuCount++] = selectWiFiSSID;
  }
  oledMenu.menuItems[menuCount] = "Back"; oledMenu.menuActions[menuCount++] = backToMain;
  oledMenu.noOfmenuItems = --menuCount;
}

void MenuManager_::locationAreaMenu() {
  resetMenu();
  menuMode = menu;
  byte menuCount = 1;
  int numberOfEntries = spiffsStorage.getZoneAreaCountFromSpiffs();

  oledMenu.menuTitle = "Location Area";
  for (int i = 0; i < numberOfEntries ; i++) {
    oledMenu.menuItems[menuCount] = spiffsStorage.getZoneAreaFromSpiffs(i); oledMenu.menuActions[menuCount++] = selectLocation;
  }
  oledMenu.menuItems[menuCount] = "Back";           oledMenu.menuActions[menuCount++] = backToMain;
  oledMenu.noOfmenuItems = --menuCount;
}

void MenuManager_::locationMenu() {
  resetMenu();
  menuMode = menu;
  byte menuCount = 1;
  int numberOfEntries = spiffsStorage.getZoneLocationCountFromSpiffs(_chosenArea);

  oledMenu.menuTitle = "Location";
  for (int i = 0; i < numberOfEntries ; i++) {
    oledMenu.menuItems[menuCount] = spiffsStorage.getZoneLocationFromSpiffs(_chosenArea, i); oledMenu.menuActions[menuCount++] = setLocation;
  }
  oledMenu.menuItems[menuCount] = "Back";           oledMenu.menuActions[menuCount++] = backToMain;
  oledMenu.noOfmenuItems = --menuCount;
}

// actions for menu selections are put in here
void MenuManager_::menuActions(menuTargets selectedAction) {
  switch (selectedAction) {
    // Top Level Menu & Management
    case noTarget: {
      break;
    }
    case unmappedOption: {
      debugMsgMnm("Unmapped option");
      mainMenu();
      break;
    }
    case backToMain: {
      mainMenu();
      break;
    }
    case gotoWifiMenu: {
      wifiMenu();
      break;
    }
    case gotoOptionsMenu: {
      systemMenu();
      break;
    }
    case gotoDisplayMenu: {
      nixieClockMenu();
      break;
    }
    case gotoTimeSetMenu: {
      setTimeMenu();
      break;
    }
    case menuOff: {
      resetMenu();
      break;
    }

    // --------------------------------------------------
    // "WiFi Menu Items"
    case reconnectPrevious: {
      wifiManager.connectToLastAP();
      wifiMenu();
      break;
    }
    case connectWPS: {
      wifiManager.connectWithWPS();
      wifiMenu();
      break;
    }
    case smartConfig: {
      wifiManager.startSmartConfig();
      wifiMenu();
      break;
    }
    case openAccessPoint: {
      wifiManager.openAccessPortal();
      wifiMenu();
      break;
    }
    case scanWiFi: {
      wifiManager.startScanWiFiNetworks();
      wifiMenu();
      break;
    }
    case selectWiFiSSID: {
      setWiFiSSIDFromSelection();
      wifiMenu();
      break;
    }
    case showWifiSelection: {
      wifiSelectMenu();
      break;
    }
    case disconnectWifi: {
      wifiManager.disconnectWiFi();
      wifiMenu();
      break;
    }
    case enterWiFiPassword: {
      setStringValue("Enter WiFi password", saveWiFiPassword, cc->WiFiPassword);
      break;
    }
    case saveWiFiPassword: {
      cc->WiFiPassword = oledMenu.enteredString;
      debugMsgMnm("Set wiFi pw: " + cc->WiFiPassword);
      wifiMenu();
      break;
    }
    case enterWiFiSSID: {
      setStringValue("Enter WiFi name", saveWiFiSSID, cc->WiFiSSID);
      break;
    }
    case saveWiFiSSID: {
      cc->WiFiSSID = oledMenu.enteredString;
      debugMsgMnm("Set SSID: " + cc->WiFiSSID);
      wifiMenu();
      break;
    }

    // --------------------------------------------------
    // "Nixie Clock Menu Items"
    case toggleTubeDimming: {
      cc->useLDRTube = ! cc->useLDRTube;
      nixieClockMenu();
      break;
    }
    case toggleBLDimming: {
      cc->useBLDim = ! cc->useBLDim;
      nixieClockMenu();
      break;
    }
    case toggleHourMode: {
      cc->hourMode = !cc->hourMode;
      nixieClockMenu();
      break;
    }
    case toggleFade: {
      cc->fade = !cc->fade;
      nixieClockMenu();
      break;
    }
    case toggleScrollback: {
      cc->scrollback = !cc->scrollback;
      nixieClockMenu();
      break;
    }
    case setDimming: {
      setDimmingValue(saveDimming);
      break;
    }
    case saveDimming: {
      if (cc->useLDRTube) {
        if (cc->minTubeDim != oledMenu.mValueEntered) {
          cc->minTubeDim = oledMenu.mValueEntered;
        }
      } else {
        if (cc->setTubeDim != oledMenu.mValueEntered) {
          cc->setTubeDim = oledMenu.mValueEntered;
        }
      }
      nixieClockMenu();
      break;
    }
    #ifdef FEATURE_BLINKENLIGHTS
    case nextBlnknMode: {
      cc->blinkenLightsMode = blinkenlightsManager.getNextBlinkenlightsMode();
      nixieClockMenu();
      break;
    }
    #endif
    case setNextSlotsMode: {
      cc->slotsMode = outputManager.getNextSlotsMode();
      nixieClockMenu();
      break;
    }
    case setNextACPMode: {
      cc->acpMode = outputManager.getNextACPMode();
      nixieClockMenu();
      break;
    }
    case setNextBlankingMode: {
      cc->dayBlanking = blankingManager.getNextBlankingMode();
      nixieClockMenu();
      break;
    }
    case setBlankingHourStart: {
      setHourValue("Blank start hour", cc->blankHourStart, saveBlankHourStart);
      break;
    }
    case saveBlankHourStart: {
      cc->blankHourStart = oledMenu.mValueEntered;
      nixieClockMenu();
      break;
    }
    case setBlankingHourEnd: {
      setHourValue("Blank end hour", cc->blankHourEnd, saveBlankHourEnd);
      break;
    }
    case saveBlankHourEnd: {
      cc->blankHourEnd = oledMenu.mValueEntered;
      nixieClockMenu();
      break;
    }

    // --------------------------------------------------
    // "System Menu Items"
    case restartClock: {
      spiffsStorage.saveStatsToSpiffs();
      flashMenuMessage("Restart","Restarting\nchronometer\ndevice now");
      delay(1000);
      ESP.restart();
      break;
    }
    case saveConfig: {
      spiffsStorage.saveConfigToSpiffs();
      systemMenu();
      break;
    }
    case saveStats: {
      spiffsStorage.saveStatsToSpiffs();
      systemMenu();
      break;
    }
    #ifdef DIGIT_DIAGNOSTICS
    case displayTest: {
      cc->diagsMode++;
      if (cc->diagsMode > DIGIT_DIAGS_MODE_MAX) {
        cc->diagsMode = DIGIT_DIAGS_MODE_MIN;
      }
      systemMenu();
      break;
    }
    #endif
    case toggleWiFiAtStart: {
      cc->WifiOnAtStart = ! cc->WifiOnAtStart;
      systemMenu();
      break;
    }
    #ifdef NIXIE_SLAVE
    case nextSlaveMode: {
      slaveManagerNixie.setNextSlaveMode();
      nixieClockMenu();
      break;
    }
    #endif
    #ifdef DEBUG
    case debugOn10mins: {
      debugManager.setDebugAutoOff(600);
      systemMenu();
      break;
    }
    #endif
    case resetWiFiInfo: {
      resetWiFi();
      systemMenu();
      break;
    }
    case selectLocationArea: {
      locationAreaMenu();
      break;
    }
    case selectLocation: {
      _chosenArea = spiffsStorage.getZoneAreaFromSpiffs(oledMenu.selectedMenuItem - 1);
      debugMsgMnm("Chose location area: " + _chosenArea);
      locationMenu();
      break;
    }
    case setLocation: {
      String _chosenLocation = spiffsStorage.getZoneLocationFromSpiffs(_chosenArea, oledMenu.selectedMenuItem - 1);
      debugMsgMnm("Chose location: " + _chosenLocation);
      String _chosenTZ = spiffsStorage.getLocationTZFromSpiffs(_chosenArea, oledMenu.selectedMenuItem - 1);
      debugMsgMnm("Chose TZ: " + _chosenTZ);
      flashMenuMessage("TZ Set","Set TZ to\n" + _chosenTZ);
      cc->tzs = _chosenTZ;
      systemMenu();
      break;
    }

    // --------------------------------------------------
    // "Manual time set"
    case setHours: {
      setHourValue("Set hour", hour(), saveHours);
      break;
    }
    case setMinutes: {
      setMinuteValue(saveMinutes);
      break;
    }
    case saveHours: {
      calculateAndSaveHourValue();
      setTimeMenu();
      break;
    }
    case saveMinutes: {
      calculateAndSaveMinuteValue();
      setTimeMenu();
      break;
    }
  }

  oledMenu.selectedMenuItem = noTarget;

    // // demonstrate selecting between 2 options only
    // if (oledMenu.selectedMenuItem == 4) {
    //   resetMenu();
    //   menuMode = value; oledMenu.menuTitle = "on or off"; oledMenu.mValueLow = 0; oledMenu.mValueHigh = 1; oledMenu.mValueStep = 1; oledMenu.mValueEntered = 0;  // set parameters
    // }

    // // demonstrate usage of 'enter a value' (non blocking)
    // if (oledMenu.selectedMenuItem == 5) {
    //   debugMsgMnm("demo_menu: none blocking enter value");
    // }

    // // demonstrate selecting between 2 options only
    // if (oledMenu.selectedMenuItem == 4) {
    //   resetMenu();
    //   menuMode = value; oledMenu.menuTitle = "on or off"; oledMenu.mValueLow = 0; oledMenu.mValueHigh = 1; oledMenu.mValueStep = 1; oledMenu.mValueEntered = 0;  // set parameters
    // }

    // // demonstrate usage of 'enter a value' (none blocking)
    // if (oledMenu.selectedMenuItem == 5) {
    //   debugMsgMnm("demo_menu: none blocking enter value");
    //   resetMenu();
    //   value1();       // enter a value
    // }

    // // demonstrate usage of 'enter a value' (blocking) which is quick and easy but stops all other tasks until the value is entered
    // if (oledMenu.selectedMenuItem == 6) {
    //   debugMsgMnm("demo_menu: blocking enter a value");
    //   // set perameters
    //     resetMenu();
    //     menuMode = value;
    //     oledMenu.menuTitle = "blocking";
    //     oledMenu.mValueLow = 0;
    //     oledMenu.mValueHigh = 50;
    //     oledMenu.mValueStep = 1;
    //     oledMenu.mValueEntered = 5;
    //   int tEntered = serviceValue(1);      // request value
    //   debugMsgMnm("The value entered was " + String(tEntered));
    //   defaultMenu();
    // }

    //  displayMessage("Message", "Please select the\noption using the\nencoder. Press down\nto select.");

}  // menuActions

//                -----------------------------------------------

void MenuManager_::setDimmingValue(menuTargets target) {
  resetMenu();                               // clear any previous menu
  menuMode = value;                          // enable value entry
  oledMenu.menuTitle = "Dim value";          // title (used to identify which number was entered)
  oledMenu.mValueLow = DIM_MIN;              // minimum value allowed
  oledMenu.mValueHigh = DIM_MAX;             // maximum value allowed
  oledMenu.mValueStep = 1;                   // step size
  if (cc->useLDRTube) {
    oledMenu.mValueEntered = cc->minTubeDim; // starting value - when using LDR
  } else {
    oledMenu.mValueEntered = cc->setTubeDim; // starting value - fixed
  }
  oledMenu.nextTarget = target;              // action to call when button pressed
}

void MenuManager_::setHourValue(String title, byte startValue, menuTargets target) {
  resetMenu();                           // clear any previous menu
  menuMode = value;                      // enable value entry
  oledMenu.menuTitle = title;            // title (used to identify which number was entered)
  oledMenu.mValueLow = 0;                // minimum value allowed
  oledMenu.mValueHigh = 23;              // maximum value allowed
  oledMenu.mValueStep = 1;               // step size
  oledMenu.mValueEntered = startValue;   // starting value
  oledMenu.nextTarget = target;          // action to call when button pressed
}

void MenuManager_::setMinuteValue(menuTargets target) {
  resetMenu();                           // clear any previous menu
  menuMode = value;                      // enable value entry
  oledMenu.menuTitle = "Set minutes";      // title (used to identify which number was entered)
  oledMenu.mValueLow = 0;                // minimum value allowed
  oledMenu.mValueHigh = 59;              // maximum value allowed
  oledMenu.mValueStep = 1;               // step size
  oledMenu.mValueEntered = minute();      // starting value
  oledMenu.nextTarget = target;          // action to call when button pressed
}

void MenuManager_::setStringValue(String title, menuTargets target, String initialValue) {
  resetMenu();                           // clear any previous menu
  menuMode = stringValue;                // enable value entry
  oledMenu.menuTitle = title;            // title (used to identify which number was entered)
  oledMenu.mValueLow = 0;                // minimum value allowed - this refers to the character set!
  oledMenu.mValueHigh = 82;              // maximum value allowed - this refers to the character set!
  oledMenu.mValueStep = 1;               // step size
  oledMenu.mValueEntered = 0;            // starting value
  oledMenu.nextTarget = target;          // action to call when button pressed
  oledMenu.enteredString = initialValue;
}

void MenuManager_::flashMenuMessage(String heading, String message) {
  // Only flash the message if the display is already on
  if (!oled.getBlanked()) {
    resetTimeouts();
    flashTimeout = FLASH_TIME;
    displayMessage(heading, message);
  }
} 

void MenuManager_::scrollMenuMessage(String message) {
  // Only show the message if the display is already on
  if (!oled.getBlanked()) {
    resetTimeouts();
    oled.showScrollingMessage(message);
  }
} 

// -------------------------------------------------------------------------------------------------
//                                         menus above here
// -------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------
//                              -loop
// ----------------------------------------------------------------
// called from main loop

void MenuManager_::menuLoop() {
  reUpdateButton();               // update rotary encoder button status (if pressed activate default menu)
  if (menuMode == off) return;    // if menu system is turned off do nothing more

  if (resetDisplay) {
    // Re-initialise
    oled.setUp();
    resetDisplay = false;
  }

  if ( configTimeout == 0 ) {
    resetMenu();
    return;
  }

  switch (menuMode) {
    case menu:
      serviceMenu();
      menuActions(oledMenu.menuActions[oledMenu.selectedMenuItem]);
      break;

    case value:
      serviceValue();
      if (rotaryEncoder.reButtonPressed) {
        debugMsgMnm("Button pressed: value: "+ String(oledMenu.mValueEntered));
        rotaryEncoder.reButtonPressed = 0;
        menuActions(oledMenu.nextTarget);
      }
      break;

    case stringValue:
      serviceValue();
      if (rotaryEncoder.reButtonPressed) {
        debugMsgMnm("Button pressed: value: "+ String(oledMenu.mValueEntered));
        rotaryEncoder.reButtonPressed = 0;
        if (oledMenu.mValueEntered == BACKSPACE) {
          oledMenu.enteredString = oledMenu.enteredString.substring(0, oledMenu.enteredString.length() - 1);
        } else if (oledMenu.mValueEntered == RESTART) {
          oledMenu.enteredString = "";
        } else if (oledMenu.mValueEntered == DONE) {
          menuActions(oledMenu.nextTarget);
        } else {
          oledMenu.enteredString = oledMenu.enteredString + CHARSET.substring(oledMenu.mValueEntered, oledMenu.mValueEntered+1);
        }
      }
      break;

    case message:
      if (rotaryEncoder.reButtonPressed == 1) mainMenu();    // if button has been pressed return to default menu
      break;

    default:
      break;
  }
}

// ----------------------------------------------------------------
//                   -button debounce (rotary encoder)
// ----------------------------------------------------------------
// update rotary encoder current button status

void MenuManager_::reUpdateButton() {
    bool tReading = digitalRead(ENC_BTN);        // read current button state
    if (tReading != rotaryEncoder.encoderPrevButton) rotaryEncoder.reLastButtonChange = nowMillis;     // if it has changed reset timer
    if ( (unsigned long)(nowMillis - rotaryEncoder.reLastButtonChange) > rotaryEncoder.reDebounceDelay ) {  // if button state is stable
      if (rotaryEncoder.encoderPrevButton == rotaryEncoder.reButtonPressedState) {
        if (rotaryEncoder.reButtonDebounced == 0) {    // if the button has been pressed
          rotaryEncoder.reButtonPressed = 1;           // flag set when the button has been pressed
          if (menuMode == off) mainMenu();             // if the display is off start the default menu
        }
        rotaryEncoder.reButtonDebounced = 1;           // debounced button status  (1 when pressed)
      } else {
        rotaryEncoder.reButtonDebounced = 0;
      }
    }

    if (rotaryEncoder.reButtonDebounced == 1) {
      resetTimeouts();
    }
    rotaryEncoder.encoderPrevButton = tReading;            // update last state read
}


// ----------------------------------------------------------------
//                       -service active menu
// ----------------------------------------------------------------

void MenuManager_::serviceMenu() {
  if (rotaryEncoder.encoder0Pos >= itemTrigger) {
    rotaryEncoder.encoder0Pos -= itemTrigger;
    oledMenu.highlightedMenuItem++;
    oledMenu.lastMenuActivity = nowMillis;
    oledMenu.needUpdate = true;
  }
  if (rotaryEncoder.encoder0Pos <= -itemTrigger) {
    rotaryEncoder.encoder0Pos += itemTrigger;
    oledMenu.highlightedMenuItem--;
    oledMenu.lastMenuActivity = nowMillis;
    oledMenu.needUpdate = true;
  }
  if (rotaryEncoder.reButtonPressed == 1) {
    oledMenu.selectedMenuItem = oledMenu.highlightedMenuItem;
    oledMenu.lastMenuActivity = nowMillis;
    oledMenu.needUpdate = true;
    debugMsgMnm("menu '" + oledMenu.menuTitle + "' item '" + oledMenu.menuItems[oledMenu.highlightedMenuItem] + "' selected");
  }

  if (oledMenu.needUpdate) {
    const int _centreLine = displayMaxLines / 2 + 1;    // mid list point
    oled.clearDisplay();
    oled.setTextColor(WHITE);

    // verify valid highlighted item
    if (oledMenu.highlightedMenuItem > oledMenu.noOfmenuItems) oledMenu.highlightedMenuItem = oledMenu.noOfmenuItems;
    if (oledMenu.highlightedMenuItem < 1) oledMenu.highlightedMenuItem = 1;

    // title
    oled.setCursor(0, 0);
    if (menuLargeText) {
      oled.setTextSize(2);
      oled.println(oledMenu.menuItems[oledMenu.highlightedMenuItem].substring(0, MaxmenuTitleLength));
    } else {
      if (oledMenu.menuTitle.length() > MaxmenuTitleLength) oled.setTextSize(1);
      else oled.setTextSize(2);
      oled.println(oledMenu.menuTitle);
    }
    oled.drawLine(0, topLine-1, oled.width(), topLine-1, WHITE);       // draw horizontal line under title

    // menu
    oled.setTextSize(1);
    oled.setCursor(0, topLine);
    for (int i=1; i <= displayMaxLines; i++) {
      int item = oledMenu.highlightedMenuItem - _centreLine + i;
      if (item == oledMenu.highlightedMenuItem) oled.setTextColor(BLACK, WHITE);
      else oled.setTextColor(WHITE);
      if (item > 0 && item <= oledMenu.noOfmenuItems) oled.println(oledMenu.menuItems[item]);
      else oled.println(" ");
    }

    oled.outputDisplay();
    oledMenu.needUpdate = false;
  }
}


// ----------------------------------------------------------------
//                        -service value entry
// ----------------------------------------------------------------
void MenuManager_::serviceValue() {
  // If we timed out, just reset
  if (configTimeout == 0) {
    resetMenu();
  }

  if (rotaryEncoder.encoder0Pos >= itemTrigger) {
    rotaryEncoder.encoder0Pos -= itemTrigger;
    oledMenu.mValueEntered-= oledMenu.mValueStep;
    oledMenu.lastMenuActivity = nowMillis;
    oledMenu.needUpdate = true;
  }
  if (rotaryEncoder.encoder0Pos <= -itemTrigger) {
    rotaryEncoder.encoder0Pos += itemTrigger;
    oledMenu.mValueEntered+= oledMenu.mValueStep;
    oledMenu.lastMenuActivity = nowMillis;
    oledMenu.needUpdate = true;
  }
  if (oledMenu.mValueEntered < oledMenu.mValueLow) {
    oledMenu.mValueEntered = oledMenu.mValueLow;
    oledMenu.lastMenuActivity = nowMillis;
    oledMenu.needUpdate = true;
  }
  if (oledMenu.mValueEntered > oledMenu.mValueHigh) {
    oledMenu.mValueEntered = oledMenu.mValueHigh;
    oledMenu.lastMenuActivity = nowMillis;
    oledMenu.needUpdate = true;
  }

  if (oledMenu.needUpdate) {
    if (menuMode == value) {
      const int _valueSpacingX = 30;      // spacing for the displayed value y position
      const int _valueSpacingY = 5;       // spacing for the displayed value y position
      oled.clearDisplay();
      oled.setTextColor(WHITE);

      // title
      oled.setCursor(0, 0);
      if (oledMenu.menuTitle.length() > MaxmenuTitleLength) oled.setTextSize(1);
      else oled.setTextSize(2);
      oled.println(oledMenu.menuTitle);
      oled.drawLine(0, topLine-1, oled.width(), topLine-1, WHITE);       // draw horizontal line under title

      // value selected
      oled.setCursor(_valueSpacingX, topLine + _valueSpacingY);
      oled.setTextSize(3);
      oled.println(String(oledMenu.mValueEntered));

      // range
      oled.setCursor(0, oled.height() - lineSpace1 - 1 );   // bottom of display
      oled.setTextSize(1);
      oled.println(String(oledMenu.mValueLow) + " to " + String(oledMenu.mValueHigh));

      // bar
      int Tlinelength = map(oledMenu.mValueEntered, oledMenu.mValueLow, oledMenu.mValueHigh, 0 , oled.width());
      oled.drawLine(0, oled.height()-1, Tlinelength, oled.height()-1, WHITE);

      oled.outputDisplay();
      oledMenu.needUpdate = false;
    } else

    if (menuMode == stringValue) {
      const int _valueSpacingY = 5;       // spacing for the displayed value y position
      oled.clearDisplay();
      oled.setTextColor(WHITE);

      // title
      oled.setCursor(0, 0);
      if (oledMenu.menuTitle.length() > MaxmenuTitleLength) oled.setTextSize(1);
      else oled.setTextSize(2);
      oled.println(oledMenu.menuTitle);
      oled.drawLine(0, topLine-1, oled.width(), topLine-1, WHITE);       // draw horizontal line under title

      // value selected
      switch (oledMenu.mValueEntered) {
        case BACKSPACE: {
          oled.setCursor(30, topLine + _valueSpacingY);
          oled.setTextSize(2);
          oled.println("DELETE");
          break;
        }
        case DONE: {
          oled.setCursor(40, topLine + _valueSpacingY);
          oled.setTextSize(2);
          oled.println("DONE");
          break;
        }
        case RESTART: {
          oled.setCursor(26, topLine + _valueSpacingY);
          oled.setTextSize(2);
          oled.println("RESTART");
          break;
        }
        default: {
          oled.setCursor(52, topLine + _valueSpacingY);
          oled.setTextSize(3);
          oled.println(CHARSET.substring(oledMenu.mValueEntered, oledMenu.mValueEntered+1));
        }
      }

      // range
      oled.setCursor(0, oled.height() - lineSpace1 - 1 );   // bottom of display
      oled.setTextSize(1);
      if (oledMenu.enteredString.length() == 0) {
        oled.println("Enter value then DONE");
      } else {
        String displayString = oledMenu.enteredString;
        int displayStrLen = oledMenu.enteredString.length();
        if (displayStrLen > 16) {
          displayString = "..." + oledMenu.enteredString.substring(displayStrLen-14, displayStrLen);
        }
        
        oled.println(String("<" + displayString + ">"));
      }
      oled.outputDisplay();
      oledMenu.needUpdate = false;

      // bar
      int Tlinelength = map(oledMenu.mValueEntered, oledMenu.mValueLow, oledMenu.mValueHigh, 0 , oled.width());
      oled.drawLine(0, oled.height()-1, Tlinelength, oled.height()-1, WHITE);

    }
  }

  reUpdateButton();        // check status of button
}


// ----------------------------------------------------------------
//                           -list create
// ----------------------------------------------------------------
// create a menu from a list
// e.g.       String tList[]={"main menu", "2", "3", "4", "5", "6"};
//            createList("demo_list", 6, &tList[0]);

void MenuManager_::createList(String _title, int _noOfElements, String *_list) {
  resetMenu();                      // clear any previous menu
  menuMode = menu;                  // enable menu mode
  oledMenu.noOfmenuItems = _noOfElements;    // set the number of items in this menu
  oledMenu.menuTitle = _title;               // menus title (used to identify it)

  for (int i=1; i <= _noOfElements; i++) {
    oledMenu.menuItems[i] = _list[i-1];        // set the menu items
  }
}


// ----------------------------------------------------------------
//                         -message display
// ----------------------------------------------------------------
// 21 characters per line, use "\n" for next line
// assistant:  <     line 1        ><     line 2        ><     line 3        ><     line 4         >

 void MenuManager_::displayMessage(String _title, String _message) {
  resetMenu();
  menuMode = message;

  oled.clearDisplay();
  oled.setTextColor(WHITE);

  // title
    oled.setCursor(0, 0);
    if (menuLargeText) {
      oled.setTextSize(2);
      oled.println(_title.substring(0, MaxmenuTitleLength));
    } else {
      if (_title.length() > MaxmenuTitleLength) oled.setTextSize(1);
      else oled.setTextSize(2);
      oled.println(_title);
    }

  // message
    oled.setCursor(0, topLine);
    oled.setTextSize(1);
    oled.println(_message);

  oled.outputDisplay();

 }

// ----------------------------------------------------------------
//                        -reset menu system
// ----------------------------------------------------------------

void MenuManager_::resetMenu() {
  // reset all menu variables / flags
  menuMode = off;
  oledMenu.selectedMenuItem = noTarget;
  rotaryEncoder.encoder0Pos = 0;
  oledMenu.noOfmenuItems = 0;
  oledMenu.menuTitle = "";
  oledMenu.highlightedMenuItem = 0;
  oledMenu.mValueEntered = 0;
  rotaryEncoder.reButtonPressed = 0;

  oledMenu.lastMenuActivity = nowMillis;

  // clear oled display
  oled.blankDisplay();
}


// ----------------------------------------------------------------
//                     -interrupt for rotary encoder
// ----------------------------------------------------------------
// rotary encoder interrupt routine to update position counter when turned
//     interrupt info: https://www.gammon.com.au/forum/bbshowpost.php?id=11488

void ICACHE_RAM_ATTR MenuManager_::doEncoder() {
  bool pinA = digitalRead(ENC_APin);
  bool pinB = digitalRead(ENC_BPin);
  int delta = 0;

  if ( (rotaryEncoder.encoderPrevA == pinA && rotaryEncoder.encoderPrevB == pinB) ) return;  // no change since last time (i.e. reject bounce)

  // same direction (alternating between 0,1 and 1,0 in one direction or 1,1 and 0,0 in the other direction)
       if (rotaryEncoder.encoderPrevA == 1 && rotaryEncoder.encoderPrevB == 0 && pinA == 0 && pinB == 1) {rotaryEncoder.encoder0Pos -= 1; delta = -1;}
  else if (rotaryEncoder.encoderPrevA == 0 && rotaryEncoder.encoderPrevB == 1 && pinA == 1 && pinB == 0) {rotaryEncoder.encoder0Pos -= 1; delta = -1;}
  else if (rotaryEncoder.encoderPrevA == 0 && rotaryEncoder.encoderPrevB == 0 && pinA == 1 && pinB == 1) {rotaryEncoder.encoder0Pos += 1; delta = 1;}
  else if (rotaryEncoder.encoderPrevA == 1 && rotaryEncoder.encoderPrevB == 1 && pinA == 0 && pinB == 0) {rotaryEncoder.encoder0Pos += 1; delta = 1;}

  // change of direction
  else if (rotaryEncoder.encoderPrevA == 1 && rotaryEncoder.encoderPrevB == 0 && pinA == 0 && pinB == 0) {rotaryEncoder.encoder0Pos += 1; delta = 1;}
  else if (rotaryEncoder.encoderPrevA == 0 && rotaryEncoder.encoderPrevB == 1 && pinA == 1 && pinB == 1) {rotaryEncoder.encoder0Pos += 1; delta = 1;}
  else if (rotaryEncoder.encoderPrevA == 0 && rotaryEncoder.encoderPrevB == 0 && pinA == 1 && pinB == 0) {rotaryEncoder.encoder0Pos -= 1; delta = -1;}
  else if (rotaryEncoder.encoderPrevA == 1 && rotaryEncoder.encoderPrevB == 1 && pinA == 0 && pinB == 1) {rotaryEncoder.encoder0Pos -= 1; delta = -1;}

  // update previous readings
  rotaryEncoder.encoderPrevA = pinA;
  rotaryEncoder.encoderPrevB = pinB;

  // Reset the display timeouts if we have movement
  resetTimeouts();

  if (menuMode == off) {
    if (delta > 0) {
      cc->towerHueOffset = cc->towerHueOffset + delta;
      cc->towerHueOffset = cc->towerHueOffset % 360;
    }

    if (delta < 0) {
      cc->hueOffset = cc->hueOffset - delta;
      cc->hueOffset = cc->hueOffset % 360;
    }
  }
}

// ----------------------------------------------------------------
//                        -utility functions
// ----------------------------------------------------------------

void MenuManager_::resetTimeouts() {
  // first press: wake up
  if (oledTimeout == 0) {
    debugMsgMnm("OLED: ON");
    resetDisplay = true;
  } else if (menuMode > off) {
    configTimeout = CONFIG_TIME;
    oledMenu.needUpdate = true;
  }
  oledTimeout = getOledTimeoutSecs(cc->oledOnTime);
}

void MenuManager_::countdownMenuTimeouts() {
  if (flashTimeout > 0) {
    flashTimeout--;
    if(flashTimeout == 0) {
      oled.clearDisplay();
      menuOncePerSecond();
    }
  }

  if (configTimeout > 0) {
    configTimeout--;
    if (configTimeout == 0) {
      oled.clearDisplay();
      menuOncePerSecond();
    }
  }

  if (oledTimeout > 0) {
    oledTimeout--;
    if (oledTimeout == 0) {
      oled.blankDisplay();
      debugMsgMnm("OLED: OFF");
    }
  }

  if ((oledTimeout == -1) && (cc->oledOnTime > 0)) {
    oledTimeout = getOledTimeoutSecs(cc->oledOnTime);
  }
}

int MenuManager_::getCurrentEncoderPos() {
  return rotaryEncoder.encoder0Pos;
}

void MenuManager_::calculateAndSaveHourValue() {
  tm nowTm = tzManager.getRTCTimeAsLocalTimeTM();
  nowTm.tm_hour = oledMenu.mValueEntered;
  time_t newTime = tzManager.convertLocalTimeTMToUTC(nowTm);
  rtcManager.setRTCTimeFromUTCSource(newTime);
  tzManager.setUTCTimeFromTimeSource(TIME_SOURCE_RTC, nowMillis, rtcManager.getRTCTimeAsTimeT());
  debugMsgMnm("New time = " + String(tzManager.gmtimeToReadableString(newTime)));
}

void MenuManager_::calculateAndSaveMinuteValue() {
  tm nowTm = tzManager.getRTCTimeAsLocalTimeTM();
  nowTm.tm_min = oledMenu.mValueEntered;
  nowTm.tm_sec = 0;
  time_t newTime = tzManager.convertLocalTimeTMToUTC(nowTm);
  rtcManager.setRTCTimeFromUTCSource(newTime);
  tzManager.setUTCTimeFromTimeSource(TIME_SOURCE_RTC, nowMillis, rtcManager.getRTCTimeAsTimeT());
  debugMsgMnm("New time = " + tzManager.gmtimeToReadableString(newTime));
}

void MenuManager_::setWiFiSSIDFromSelection() {
  debugMsgMnm("Selected option = " + String(oledMenu.selectedMenuItem));
  String selectedWiFi = wifiManager.getLastScanResultSSID(oledMenu.selectedMenuItem - 1);
  debugMsgMnm("Selected WiFi = " + selectedWiFi);
  cc->WiFiSSID = selectedWiFi;
}

// ----------------------------------------------------------------
//                              -hooks
// ----------------------------------------------------------------

// ************************************************************
// Build the status display
// ************************************************************
void MenuManager_::menuOncePerSecond() {
  // Manage timeouts
  countdownMenuTimeouts();

  if (oledTimeout != 0 && configTimeout == 0 && flashTimeout == 0) {
    oled.showStatusLine();
    // Show the info menu
    char time_c[11];
    sprintf(time_c, "%02d:%02d:%02d", hour(), minute(), second());
    oled.setTimeString(String(time_c));

    oled.clearScrollingMessage();
    if (WiFi.isConnected()) {
      oled.showScrollingMessage("IP: " + WiFi.localIP().toString());
      oled.showScrollingMessage(String(WiFi.getHostname()) + ".local");
      oled.showScrollingMessage(String(WiFi.SSID()));
    } else {
      oled.showScrollingMessage("WiFi not connected");
    }
  }
}

// ************************************************************
// Things that need updating once per hour
// ************************************************************
void MenuManager_::menuOncePerHour() {
  // nothing at present
}

// ************************************************************
// Get the current blanking status of the OLED
// ************************************************************
bool MenuManager_::getOledIsBlanked() {
  return oledTimeout == 0;
}

// ************************************************************
// Get the current timout value based on the menu setting
// ************************************************************
int MenuManager_::getOledTimeoutSecs(byte oledTimeoutSetting) {
  switch (oledTimeoutSetting) {
    case OLED_ON_ALWAYS:
      return OLED_ON_TIME_ON;
      break;
    case OLED_ON_SHORT:
      return OLED_ON_TIME_SHORT;
      break;
    case OLED_ON_LONG:
      return OLED_ON_TIME_LONG;
      break;
    default:
      return OLED_ON_TIME_SHORT;
      break;
  }
}

// ----------------------------------------------------------------
//                        -internal plumbing
// ----------------------------------------------------------------
void IRAM_ATTR doEncoderWrapper() {
  portENTER_CRITICAL_ISR(&encoderMux);
  menuManager.doEncoder();
  portEXIT_CRITICAL_ISR(&encoderMux);
}

void MenuManager_::setupMenuManager() {
  pinMode(ENC_BTN, INPUT_PULLUP);
  pinMode(ENC_APin, INPUT);
  pinMode(ENC_BPin, INPUT);

  // Interrupt for reading the rotary encoder position
  rotaryEncoder.encoder0Pos = 0;
  attachInterrupt(digitalPinToInterrupt(ENC_APin), doEncoderWrapper, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_BPin), doEncoderWrapper, CHANGE);
}

// ************************************************************
// Library internal singleton wiring
// ************************************************************
MenuManager_ &MenuManager_::getInstance() {
  static MenuManager_ instance;
  return instance;
}

MenuManager_ &menuManager = menuManager.getInstance();