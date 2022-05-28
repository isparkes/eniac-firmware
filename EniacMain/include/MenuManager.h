#pragma once

#include <Arduino.h>
#include "Defs.h"
#include "OLED.h"
#include "DebugManager.h"
#include "utilities.h"
#include "WiFiManager.h"

#define BUTTONPRESSEDSTATE 0              // rotary encoder gpio pin logic level when the button is pressed (usually 0)
#define DEBOUNCEDELAY 100                 // debounce delay for button inputs
#define maxmenuItems 12                   // max number of items used in any of the menus (keep as low as possible to save memory)

enum menuTargets {
  noTarget,
  unmappedOption,

  // Move around in menus
  backToMain,
  gotoWifiMenu,
  gotoOptionsMenu,
  gotoDisplayMenu,
  gotoTimeSetMenu,
  menuOff,

  toggleWiFiAtStart,
  disconnectWifi,
  resetWiFiInfo,
  connectWPS,
  reconnectPrevious,
  openAccessPoint,
  smartConfig,
  scanWiFi,
  showWifiSelection,
  selectWiFiSSID,
  enterWiFiPassword,
  saveWiFiPassword,
  enterWiFiSSID,
  saveWiFiSSID,
  
  toggleTubeDimming,
  toggleBLDimming,
  setDimming, saveDimming,
  nextBlnknMode,

  toggleHourMode,
  toggleFade,
  toggleScrollback,
  setNextACPMode,
  setNextSlotsMode,

  setHours, saveHours,
  setMinutes, saveMinutes,
  restartClock,
  saveStats,
  saveConfig,
  displayTest,
  nextSlaveMode,
  debugOn10mins,
  selectLocationArea,
  selectLocation,
  setLocation
};

// modes that the menu system can be in
enum menuModes {
  off,                                  // display is off
  menu,                                 // a menu is active
  value,                                // 'enter a numeric value' non blocking is active
  stringValue,                          // 'enter a string value' non blocking is active
  message,                              // displaying a message
  blocking                              // a blocking procedure is in progress (see enter value)
};


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
  bool needUpdate;                          // If the menu changed and needs to be output again
  String enteredString = "";
};

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

const bool menuLargeText = 0;             // show larger text when possible (if struggling to read the small text)
const int itemTrigger = 2;                // rotary encoder - counts per tick (varies between encoders usually 1 or 2)
const int topLine = 18;                   // y position of lower area of the display (18 with two colour displays)
const byte lineSpace1 = 9;                // line spacing for textsize 1 (small text)
const byte lineSpace2 = 17;               // line spacing for textsize 2 (large text)
const int displayMaxLines = 5;            // max lines that can be displayed in lower section of display in textsize1 (5 on larger oLeds)
const int MaxmenuTitleLength = 10;        // max characters per line when using text size 2 (usually 10)

const String CHARSET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+@#*%&/()=?!$-_ ";
const int BACKSPACE = 80;
const int DONE = 81;
const int RESTART = 82;

class MenuManager_ {
  private:
    MenuManager_() = default; // Make constructor private

  public:
    static MenuManager_ &getInstance(); // Accessor for singleton instance

    MenuManager_(const MenuManager_ &) = delete; // no copying
    MenuManager_ &operator=(const MenuManager_ &) = delete;

  public:
    void setupMenuManager();
    void ICACHE_RAM_ATTR doEncoder();
    void flashMenuMessage(String heading, String message);
    void scrollMenuMessage(String message);
    int  getCurrentEncoderPos();
    void menuOncePerSecond();
    void menuOncePerHour();
    void menuLoop();
  private:
    menuModes menuMode = off;                 // default mode at startup is off
    oledMenus oledMenu;
    rotaryEncoders rotaryEncoder;
    String _chosenArea;

    // trigger for Oled reset
    bool resetDisplay;

    // Menus
    void mainMenu();
    void wifiMenu();
    void systemMenu();
    void nixieClockMenu();
    void wifiSelectMenu();
    void setTimeMenu();
    void locationAreaMenu();
    void locationMenu();

    void menuActions(menuTargets selectedAction);
    void setDimmingValue(menuTargets target);
    void setHourValue(menuTargets target);
    void setMinuteValue(menuTargets target);
    void setStringValue(String title, menuTargets target, String intialValue);

    void serviceMenu();
    void serviceValue();
    void menuValues();
    void reUpdateButton();
    void createList(String _title, int _noOfElements, String *_list);
    void displayMessage(String _title, String _message);
    void resetMenu();
    void resetTimeouts();
    void countdownMenuTimeouts();
    void manageMenu();

    void calculateAndSaveHourValue();
    void calculateAndSaveMinuteValue();
    void setWiFiSSIDFromSelection();
};

extern MenuManager_ &menuManager;
