#pragma once

#include <Arduino.h>
#include "defs.h"
#include "OLED.h"
#include "DebugManager.h"
#include "globals.h"
#include "utilities.h"
#include "WiFiManager.h"

  // Misc
  #define BUTTONPRESSEDSTATE 0              // rotary encoder gpio pin logic level when the button is pressed (usually 0)
  #define DEBOUNCEDELAY 100                 // debounce delay for button inputs
  const bool menuLargeText = 0;             // show larger text when possible (if struggling to read the small text)
  const int maxmenuItems = 12;              // max number of items used in any of the menus (keep as low as possible to save memory)
  const int itemTrigger = 2;                // rotary encoder - counts per tick (varies between encoders usually 1 or 2)
  const int topLine = 18;                   // y position of lower area of the display (18 with two colour displays)
  const byte lineSpace1 = 9;                // line spacing for textsize 1 (small text)
  const byte lineSpace2 = 17;               // line spacing for textsize 2 (large text)
  const int displayMaxLines = 5;            // max lines that can be displayed in lower section of display in textsize1 (5 on larger oLeds)
  const int MaxmenuTitleLength = 10;        // max characters per line when using text size 2 (usually 10)

  // forward declarations
  void ICACHE_RAM_ATTR doEncoder();
  void setupMenuManager();
  void serviceMenu();
  void menuLoop();
  void menuActions();
  void value1();
  void menuValues();
  void reUpdateButton();
  void serviceMenu();
  int  serviceValue(bool _blocking);
  void createList(String _title, int _noOfElements, String *_list);
  void displayMessage(String _title, String _message);
  void resetMenu();
  void debugMsgMM(String message);
  void resetTimeouts();
  void countdownMenuTimeouts();
  void flashMenuMessage(String heading, String message);
  void manageMenu();
  int getCurrentEncoderPos();
  void menuOncePerSecond();
  void menuOncePerHour();
