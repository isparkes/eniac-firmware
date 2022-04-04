#include "MenuManager.h"

enum menuTargets {
  noTarget,
  unmappedOption,

  // Move around in menus
  backToMain,
  gotoWifiMenu,
  gotoOptionsMenu,
  gotoDisplayMenu,
  menuOff,

  toggleWiFiAtStart,
  disconnectWifi,
  resetWiFiInfo,
  connectWPS,
  reconnectPrevious,
  openAccessPoint,
  getSSIDList,
  smartConfig,
  scanWiFi,
  showWifiSelection,
  
  toggleTubeDimming,
  toggleBLDimming,
  setDimming,
  saveDimming,
  nextBlnknMode,

  restartClock,
  saveStats,
  displayTest,
  startSlave,
  stopSlave,
  toggleHourMode,
  toggleFade,
  toggleScrollback
};

// modes that the menu system can be in
enum menuModes {
  off,                                  // display is off
  menu,                                 // a menu is active
  value,                                // 'enter a value' none blocking is active
  message,                              // displaying a message
  blocking                              // a blocking procedure is in progress (see enter value)
};
menuModes menuMode = off;                 // default mode at startup is off

struct oledMenus {
  // menu
  String menuTitle = "";                    // the title of active mode
  int noOfmenuItems = 0;                    // number if menu items in the active menu
  int selectedMenuItem = 0;                 // when a menu item is selected it is flagged here until actioned and cleared
  int highlightedMenuItem = 0;              // which item is curently highlighted in the menu
  String menuItems[maxmenuItems+1];         // store for the menu item titles
  menuTargets menuActions[maxmenuItems+1];  // The action to carry out
  uint32_t lastMenuActivity = 0;            // time the menu last saw any activity (used for timeout)

  // 'enter a value'
  int mValueEntered = 0;                    // store for number entered by value entry menu
  int mValueLow = 0;                        // lowest allowed value
  int mValueHigh = 0;                       // highest allowed value
  int mValueStep = 0;                       // step size when encoder is turned
  menuTargets nextTarget = noTarget;        // the target to continue when a value is received
};
oledMenus oledMenu;

struct rotaryEncoders {
  volatile int encoder0Pos = 0;                          // current value selected with rotary encoder (updated by interrupt routine)
  volatile bool encoderPrevA;                            // used to debounced rotary encoder
  volatile bool encoderPrevB;                            // used to debounced rotary encoder
  uint32_t reLastButtonChange = 0;                       // last time state of button changed (for debouncing)
  bool encoderPrevButton = 0;                            // used to debounce button
  int reButtonDebounced = 0;                             // debounced current button state (1 when pressed)
  const bool reButtonPressedState = BUTTONPRESSEDSTATE;  // the logic level when the button is pressed
  const uint32_t reDebounceDelay = DEBOUNCEDELAY;        // button debounce delay setting
  bool reButtonPressed = 0;                              // flag set when the button is pressed (it has to be manually reset)
};
rotaryEncoders rotaryEncoder;

// Private fwd decls
void setDimmingValue(menuTargets nextAction);
void serviceMenu();
void serviceValue();
void menuActions(menuTargets target);

// trigger for Oled reset
static bool resetDisplay;

// -------------------------------------------------------------------------------------------------
//                                         menus below here
// -------------------------------------------------------------------------------------------------

void mainMenu() {
  resetMenu();
  byte menuCount = 1;
  menuMode = menu;
//  oledMenu.menuId = main;
  oledMenu.menuTitle = "Main Menu";
  oledMenu.menuItems[menuCount] = "Wifi";       oledMenu.menuActions[menuCount++] = gotoWifiMenu;
  oledMenu.menuItems[menuCount] = "Display";    oledMenu.menuActions[menuCount++] = gotoDisplayMenu;
  oledMenu.menuItems[menuCount] = "Options";    oledMenu.menuActions[menuCount++] = gotoOptionsMenu;
  oledMenu.menuItems[menuCount] = "Menu Off";   oledMenu.menuActions[menuCount++] = menuOff;
  oledMenu.noOfmenuItems = --menuCount;
}

void wifiMenu() {
  resetMenu();
  menuMode = menu;
  String onOffMsg;
  if (cc->WifiOnAtStart) {
    onOffMsg = "WiFi off at start";
  } else {
    onOffMsg = "WiFi on at start";
  }
  byte menuCount = 1;
  if (WiFi.isConnected()) {
    oledMenu.menuTitle = "WiFi Menu";           
    oledMenu.menuItems[menuCount] = onOffMsg;             oledMenu.menuActions[menuCount++] = toggleWiFiAtStart;
    oledMenu.menuItems[menuCount] = "Disconnect WiFi";    oledMenu.menuActions[menuCount++] = disconnectWifi;
    oledMenu.menuItems[menuCount] = "Reset WiFi";         oledMenu.menuActions[menuCount++] = resetWiFiInfo;
    oledMenu.menuItems[menuCount] = "Back";               oledMenu.menuActions[menuCount++] = backToMain;
  } else {
    oledMenu.noOfmenuItems = 8;
    oledMenu.menuTitle = "WiFi Menu";
    if (wifiCredentialsReceived()) {
      oledMenu.menuItems[menuCount] = "Reconnect previous"; oledMenu.menuActions[menuCount++] = reconnectPrevious;
    }
    oledMenu.menuItems[menuCount] = "Connect with WPS";   oledMenu.menuActions[menuCount++] = connectWPS;
    oledMenu.menuItems[menuCount] = "Start SmartConfig";  oledMenu.menuActions[menuCount++] = smartConfig;
    oledMenu.menuItems[menuCount] = "Open Access Point";  oledMenu.menuActions[menuCount++] = openAccessPoint;
    oledMenu.menuItems[menuCount] = "Select SSID";        oledMenu.menuActions[menuCount++] = getSSIDList;
    oledMenu.menuItems[menuCount] = "Enter password";     oledMenu.menuActions[menuCount++] = unmappedOption;
    oledMenu.menuItems[menuCount] = onOffMsg;             oledMenu.menuActions[menuCount++] = toggleWiFiAtStart;
    oledMenu.menuItems[menuCount] = "Scan Wifi";          oledMenu.menuActions[menuCount++] = scanWiFi;
    oledMenu.menuItems[menuCount] = "Select WiFi";        oledMenu.menuActions[menuCount++] = showWifiSelection;
    oledMenu.menuItems[menuCount] = "reset WiFi";         oledMenu.menuActions[menuCount++] = resetWiFiInfo;
    oledMenu.menuItems[menuCount] = "Back";               oledMenu.menuActions[menuCount++] = backToMain;
  }
  oledMenu.noOfmenuItems = --menuCount;
}

void optionsMenu() {
  resetMenu();
  menuMode = menu;
  byte menuCount = 1;
  oledMenu.menuTitle = "Options";
  oledMenu.menuItems[menuCount] = "Restart Device"; oledMenu.menuActions[menuCount++] = restartClock;
  oledMenu.menuItems[menuCount] = "Save stats";     oledMenu.menuActions[menuCount++] = saveStats;
  oledMenu.menuItems[menuCount] = "Display Test";   oledMenu.menuActions[menuCount++] = displayTest;
  oledMenu.menuItems[menuCount] = "Back";           oledMenu.menuActions[menuCount++] = backToMain;
  oledMenu.noOfmenuItems = --menuCount;
}

void displayMenu() {
  resetMenu();
  menuMode = menu;
  byte menuCount = 1;
  oledMenu.menuTitle = "display";
  oledMenu.menuItems[menuCount] = "Tube Dimming on/off";        oledMenu.menuActions[menuCount++] = toggleTubeDimming;
  oledMenu.menuItems[menuCount] = "BL Dimming on/off";          oledMenu.menuActions[menuCount++] = toggleBLDimming;
  if (cc->hourMode) {
    oledMenu.menuItems[menuCount] = "Set 24h mode";             oledMenu.menuActions[menuCount++] = toggleHourMode;
  } else {
    oledMenu.menuItems[menuCount] = "Set 12h mode";             oledMenu.menuActions[menuCount++] = toggleHourMode;
  }
  if (cc->fade) {
    oledMenu.menuItems[menuCount] = "Digit fade off";           oledMenu.menuActions[menuCount++] = toggleFade;
  } else {
    oledMenu.menuItems[menuCount] = "Digit fade on";            oledMenu.menuActions[menuCount++] = toggleFade;
  }
  if (cc->scrollback) {
    oledMenu.menuItems[menuCount] = "Scrollback off";           oledMenu.menuActions[menuCount++] = toggleScrollback;
  } else {
    oledMenu.menuItems[menuCount] = "Scrollback on";            oledMenu.menuActions[menuCount++] = toggleScrollback;
  }
  oledMenu.menuItems[menuCount] = "Set Dimming value";          oledMenu.menuActions[menuCount++] = setDimming;
  String nextBLModeName = blinkenlightsManager.getNextBlinkenlightsModeName(cc->blinkenLightsMode);
  oledMenu.menuItems[menuCount] = "BL mode: " + nextBLModeName; oledMenu.menuActions[menuCount++] = nextBlnknMode;
  if (slaveManager.getSlaveMode()) {
    oledMenu.menuItems[menuCount] = "Stop slave";               oledMenu.menuActions[menuCount++] = stopSlave;
  } else {
    oledMenu.menuItems[menuCount] = "Start slave";              oledMenu.menuActions[menuCount++] = startSlave;
  }
  oledMenu.menuItems[menuCount] = "Back";                       oledMenu.menuActions[menuCount++] = backToMain;
  oledMenu.noOfmenuItems = --menuCount;
}

void wifiSelectMenu() {
  resetMenu();
  menuMode = menu;
  byte menuCount = 1;
  oledMenu.menuTitle = "Select network";

  debugMsgMnm("Last result: " + String(getLastScanResultCount()));

  for (int i = 0; i < getLastScanResultCount() ; i++) {
    oledMenu.menuItems[menuCount] = getLastScanResultSSID(i);   oledMenu.menuActions[menuCount++] = startSlave;
  }
  oledMenu.menuItems[menuCount] = "Back";                       oledMenu.menuActions[menuCount++] = backToMain;
  oledMenu.noOfmenuItems = --menuCount;
}

// actions for menu selections are put in here
void menuActions(menuTargets selectedAction) {
  switch (selectedAction) {
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
      optionsMenu();
      break;
    }
    case gotoDisplayMenu: {
      displayMenu();
      break;
    }
    case menuOff: {
      resetMenu();
      break;
    }
    case toggleWiFiAtStart: {
      cc->WifiOnAtStart = ! cc->WifiOnAtStart;
      spiffsStorage.saveConfigToSpiffs(cc);
      wifiMenu();
      break;
    }
    case getSSIDList: {
      startScanWiFiNetworks();
      wifiMenu();
      break;
    }
    case smartConfig: {
      startSmartConfig();
      wifiMenu();
      break;
    }
    case disconnectWifi: {
      disconnectWiFi();
      wifiMenu();
      break;
    }
    case resetWiFiInfo: {
      resetWiFi();
      wifiMenu();
      break;
    }
    case scanWiFi: {
      startScanWiFiNetworks();
      wifiMenu();
      break;
    }
    case showWifiSelection: {
      wifiSelectMenu();
      break;
    }
    case connectWPS: {
      connectWithWPS();
      wifiMenu();
      break;
    }
    case reconnectPrevious: {
      connectToLastAP();
      wifiMenu();
      break;
    }
    case openAccessPoint: {
      openAccessPortal();
      wifiMenu();
      break;
    }
    case toggleTubeDimming: {
      cc->useLDR = ! cc->useLDR;
      spiffsStorage.saveConfigToSpiffs(cc);
      displayMenu();
      break;
    }
    case toggleBLDimming: {
      cc->useBLDim = ! cc->useBLDim;
      spiffsStorage.saveConfigToSpiffs(cc);
      displayMenu();
      break;
    }
    case setDimming: {
      setDimmingValue(saveDimming);
      break;
    }
    case saveDimming: {
      if (cc->minDim != oledMenu.mValueEntered) {
        cc->minDim = oledMenu.mValueEntered;
        spiffsStorage.saveConfigToSpiffs(cc);
        displayMenu();
      }
      break;
    }
    case nextBlnknMode: {
      cc->blinkenLightsMode = blinkenlightsManager.getNextBlinkenlightsMode(cc->blinkenLightsMode);
      spiffsStorage.saveConfigToSpiffs(cc);
      displayMenu();
      break;
    }
    case saveStats: {
      spiffsStorage.saveStatsToSpiffs(cs);
      break;
    }
    case restartClock: {
      spiffsStorage.saveStatsToSpiffs(cs);
      flashMenuMessage("Restart","Restarting\nchronometer\ndevice now");
      delay(1000);
      ESP.restart();
      break;
    }
    case displayTest: {
      cc->diagsMode++;
      if (cc->diagsMode > DIGIT_DIAGS_MODE_MAX) {
        cc->diagsMode = DIGIT_DIAGS_MODE_MIN;
      }
      break;
    }
    case startSlave: {
      slaveManager.startSlaveI2C();
      displayMenu();
      break;
    }
    case stopSlave: {
      slaveManager.stopSlaveI2C();
      displayMenu();
      break;
    }
    case toggleHourMode: {
      cc->hourMode = !cc->hourMode;
      spiffsStorage.saveConfigToSpiffs(cc);
      displayMenu();
      break;
    }
    case toggleFade: {
      cc->fade = !cc->fade;
      spiffsStorage.saveConfigToSpiffs(cc);
      displayMenu();
      break;
    }
    case toggleScrollback: {
      cc->scrollback = !cc->scrollback;
      spiffsStorage.saveConfigToSpiffs(cc);
      displayMenu();
      break;
    }
  }

  oledMenu.selectedMenuItem = noTarget;

    // // demonstrate selecting between 2 options only
    // if (oledMenu.selectedMenuItem == 4) {
    //   resetMenu();
    //   menuMode = value; oledMenu.menuTitle = "on or off"; oledMenu.mValueLow = 0; oledMenu.mValueHigh = 1; oledMenu.mValueStep = 1; oledMenu.mValueEntered = 0;  // set parameters
    // }

    // // demonstrate usage of 'enter a value' (none blocking)
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


// demonstration enter a value
void setDimmingValue(menuTargets target) {
  resetMenu();                           // clear any previous menu
  menuMode = value;                      // enable value entry
  oledMenu.menuTitle = "Dim value";      // title (used to identify which number was entered)
  oledMenu.mValueLow = MIN_DIM_MIN;      // minimum value allowed
  oledMenu.mValueHigh = MIN_DIM_MAX;     // maximum value allowed
  oledMenu.mValueStep = 1;               // step size
  oledMenu.mValueEntered = cc->minDim;   // starting value
  oledMenu.nextTarget = target;          // action to call when button pressed
}


// actions for value entered put in here
void menuValues() {
  // action for "demo_value"
  if (oledMenu.menuTitle == "demo_value") {
    String tString = String(oledMenu.mValueEntered);
    debugMsgMnm("demo_value: The value entered was " + tString);
    displayMessage("ENTERED", "\nYou entered\nthe value\n    " + tString);
    // alternatively use 'resetMenu()' here to turn menus off after value entered - or use 'defaultMenu()' to re-start the default menu
  }

  // action for "on or off"
  if (oledMenu.menuTitle == "on or off") {
    debugMsgMnm("demo_menu: on off selection was " + String(oledMenu.mValueEntered));
    mainMenu();
  }
}

void flashMenuMessage(String heading, String message) {
  resetTimeouts();
  flashTimeout = FLASH_TIME;
  displayMessage(heading, message);  
} 


// -------------------------------------------------------------------------------------------------
//                                         menus above here
// -------------------------------------------------------------------------------------------------


// ----------------------------------------------------------------
//                              -setup
// ----------------------------------------------------------------
// called from main setup

void setupMenuManager() {

  // configure gpio pins for rotary encoder
  pinMode(ENC_BTN, INPUT_PULLUP);
  pinMode(ENC_APin, INPUT);
  pinMode(ENC_BPin, INPUT);

  // Interrupt for reading the rotary encoder position
  rotaryEncoder.encoder0Pos = 0;
  attachInterrupt(digitalPinToInterrupt(ENC_APin), doEncoder, CHANGE);
}


// ----------------------------------------------------------------
//                              -loop
// ----------------------------------------------------------------
// called from main loop

void menuLoop() {
  reUpdateButton();               // update rotary encoder button status (if pressed activate default menu)
  if (menuMode == off) return;    // if menu system is turned off do nothing more

  if (resetDisplay) {
    // Re-initialise
    oled.setUp();
    resetDisplay = false;
  }

// debugMsgMnm("Mode: " + String(menuMode));
  // if no recent activity then turn oled off
    if ( configTimeout == 0 ) {
      resetMenu();
      return;
    }

    switch (menuMode) {
      // if there is an active menu
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

      case message:
        if (rotaryEncoder.reButtonPressed == 1) mainMenu();    // if button has been pressed return to default menu
        break;

      default:
        break;
    }
}  // menuLoop

// ----------------------------------------------------------------
//                   -button debounce (rotary encoder)
// ----------------------------------------------------------------
// update rotary encoder current button status

void reUpdateButton() {
    bool tReading = digitalRead(ENC_BTN);        // read current button state
    if (tReading != rotaryEncoder.encoderPrevButton) rotaryEncoder.reLastButtonChange = nowMillis;     // if it has changed reset timer
    if ( (unsigned long)(nowMillis - rotaryEncoder.reLastButtonChange) > rotaryEncoder.reDebounceDelay ) {  // if button state is stable
      if (rotaryEncoder.encoderPrevButton == rotaryEncoder.reButtonPressedState) {
        if (rotaryEncoder.reButtonDebounced == 0) {    // if the button has been pressed
          rotaryEncoder.reButtonPressed = 1;           // flag set when the button has been pressed
          if (menuMode == off) mainMenu();          // if the display is off start the default menu
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

void serviceMenu() {
  bool needUpdate = false;

  if (rotaryEncoder.encoder0Pos >= itemTrigger) {
    rotaryEncoder.encoder0Pos -= itemTrigger;
    oledMenu.highlightedMenuItem++;
    oledMenu.lastMenuActivity = nowMillis;
    needUpdate = true;
  }
  if (rotaryEncoder.encoder0Pos <= -itemTrigger) {
    rotaryEncoder.encoder0Pos += itemTrigger;
    oledMenu.highlightedMenuItem--;
    oledMenu.lastMenuActivity = nowMillis;
    needUpdate = true;
  }
  if (rotaryEncoder.reButtonPressed == 1) {
    oledMenu.selectedMenuItem = oledMenu.highlightedMenuItem;     // flag that the item has been selected
    oledMenu.lastMenuActivity = nowMillis;
    needUpdate = true;
    debugMsgMnm("menu '" + oledMenu.menuTitle + "' item '" + oledMenu.menuItems[oledMenu.highlightedMenuItem] + "' selected");
  }

  if (needUpdate) {
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
  }
}


// ----------------------------------------------------------------
//                        -service value entry
// ----------------------------------------------------------------
void serviceValue() {
  // If we timed out, just reset
  if (configTimeout == 0) {
    resetMenu();
  }

  bool needUpdate = false;

  // rotary encoder
  if (rotaryEncoder.encoder0Pos >= itemTrigger) {
    rotaryEncoder.encoder0Pos -= itemTrigger;
    oledMenu.mValueEntered-= oledMenu.mValueStep;
    oledMenu.lastMenuActivity = nowMillis;
    needUpdate = true;
  }
  if (rotaryEncoder.encoder0Pos <= -itemTrigger) {
    rotaryEncoder.encoder0Pos += itemTrigger;
    oledMenu.mValueEntered+= oledMenu.mValueStep;
    oledMenu.lastMenuActivity = nowMillis;
    needUpdate = true;
  }
  if (oledMenu.mValueEntered < oledMenu.mValueLow) {
    oledMenu.mValueEntered = oledMenu.mValueLow;
    oledMenu.lastMenuActivity = nowMillis;
    needUpdate = true;
  }
  if (oledMenu.mValueEntered > oledMenu.mValueHigh) {
    oledMenu.mValueEntered = oledMenu.mValueHigh;
    oledMenu.lastMenuActivity = nowMillis;
    needUpdate = true;
  }

  if (needUpdate) {
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
  }

  reUpdateButton();        // check status of button
}


// ----------------------------------------------------------------
//                           -list create
// ----------------------------------------------------------------
// create a menu from a list
// e.g.       String tList[]={"main menu", "2", "3", "4", "5", "6"};
//            createList("demo_list", 6, &tList[0]);

void createList(String _title, int _noOfElements, String *_list) {
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

 void displayMessage(String _title, String _message) {
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

void resetMenu() {
  // reset all menu variables / flags
  menuMode = off;
  oledMenu.selectedMenuItem = noTarget;
  rotaryEncoder.encoder0Pos = 0;
  oledMenu.noOfmenuItems = 0;
  oledMenu.menuTitle = "";
  oledMenu.highlightedMenuItem = 0;
  oledMenu.mValueEntered = 0;
  rotaryEncoder.reButtonPressed = 0;

  oledMenu.lastMenuActivity = nowMillis;   // log time

  // clear oled display
  oled.blankDisplay();
}


// ----------------------------------------------------------------
//                     -interrupt for rotary encoder
// ----------------------------------------------------------------
// rotary encoder interrupt routine to update position counter when turned
//     interrupt info: https://www.gammon.com.au/forum/bbshowpost.php?id=11488

void ICACHE_RAM_ATTR doEncoder() {
  bool pinA = digitalRead(ENC_APin);
  bool pinB = digitalRead(ENC_BPin);

  if ( (rotaryEncoder.encoderPrevA == pinA && rotaryEncoder.encoderPrevB == pinB) ) return;  // no change since last time (i.e. reject bounce)

  // same direction (alternating between 0,1 and 1,0 in one direction or 1,1 and 0,0 in the other direction)
       if (rotaryEncoder.encoderPrevA == 1 && rotaryEncoder.encoderPrevB == 0 && pinA == 0 && pinB == 1) rotaryEncoder.encoder0Pos -= 1;
  else if (rotaryEncoder.encoderPrevA == 0 && rotaryEncoder.encoderPrevB == 1 && pinA == 1 && pinB == 0) rotaryEncoder.encoder0Pos -= 1;
  else if (rotaryEncoder.encoderPrevA == 0 && rotaryEncoder.encoderPrevB == 0 && pinA == 1 && pinB == 1) rotaryEncoder.encoder0Pos += 1;
  else if (rotaryEncoder.encoderPrevA == 1 && rotaryEncoder.encoderPrevB == 1 && pinA == 0 && pinB == 0) rotaryEncoder.encoder0Pos += 1;

  // change of direction
  else if (rotaryEncoder.encoderPrevA == 1 && rotaryEncoder.encoderPrevB == 0 && pinA == 0 && pinB == 0) rotaryEncoder.encoder0Pos += 1;
  else if (rotaryEncoder.encoderPrevA == 0 && rotaryEncoder.encoderPrevB == 1 && pinA == 1 && pinB == 1) rotaryEncoder.encoder0Pos += 1;
  else if (rotaryEncoder.encoderPrevA == 0 && rotaryEncoder.encoderPrevB == 0 && pinA == 1 && pinB == 0) rotaryEncoder.encoder0Pos -= 1;
  else if (rotaryEncoder.encoderPrevA == 1 && rotaryEncoder.encoderPrevB == 1 && pinA == 0 && pinB == 1) rotaryEncoder.encoder0Pos -= 1;

  //else if (serialDebug) Serial.println("Error: invalid rotary encoder pin state - prev=" + String(rotaryEncoder.encoderPrevA) + ","
  //                                      + String(rotaryEncoder.encoderPrevB) + " new=" + String(pinA) + "," + String(pinB));

  // update previous readings
  rotaryEncoder.encoderPrevA = pinA;
  rotaryEncoder.encoderPrevB = pinB;
  resetTimeouts();

  // if (menuMode == off) {
  //   ledManager.setTowerHueOffset(rotaryEncoder.encoder0Pos);
  // }
}

// ---------------------------------------------- end ----------------------------------------------

void resetTimeouts() {
  // first press: wake up
  if (oledTimeout == 0) {
    debugMsgMnm("OLED: ON");
    resetDisplay = true;
  } else if (menuMode > off) {
    configTimeout = CONFIG_TIME;
  }
  oledTimeout = OLED_ON_TIME;
}

void countdownMenuTimeouts() {
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
}

int getCurrentEncoderPos() {
  return rotaryEncoder.encoder0Pos;
}

// ************************************************************
// Build the status display
// ************************************************************
void menuOncePerSecond() {
  if (oledTimeout > 0 && configTimeout == 0 && flashTimeout == 0) {
    oled.showStatusLine();
    // Show the info menu
    char time_c[11];
    sprintf(time_c, "%02d:%02d:%02d", hour(), minute(), second());
    oled.setTimeString(String(time_c));

    oled.setWiFiStatus(WiFi.isConnected());
    oled.setNTPStatus(ntpManager.ntpTimeValid());
    oled.setGPSStatus(gpsManager.getGPSTimeValid());
    oled.setBlankStatus(false);
    if (digitalRead(PIRPin) == false) {
      oled.setPIRInstalled(true);  
    }
    oled.setPIRStatus(digitalRead(PIRPin));
    oled.setBTN1Status(digitalRead(BTN1Pin) == LOW);
    oled.setBTN2Status(digitalRead(BTN2Pin) == LOW);

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

void menuOncePerHour() {
  if (oledTimeout > 0 && configTimeout == 0 && flashTimeout == 0) {
    oled.setAMStatus(isAM());
  }
}
