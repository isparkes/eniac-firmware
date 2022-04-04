#include "DebugManager.h"
#include <Arduino.h>

void DebugManager_::begin() {
}

// ************************************************************
// Output a logging message to the debug output, if set
// ************************************************************
void DebugManager_::debugMsg(String message) {
    Serial.println(message);
    Serial.flush();
}

// ************************************************************
// Output a logging message to the debug output, if set, 
// adding prefix
// ************************************************************
void DebugManager_::debugMsg(String prefix, String message) {
    Serial.println(prefix + ": " + message);
    Serial.flush();
}

void DebugManager_::debugMsgCont(String message) {
    Serial.print(message);
    Serial.flush();
}

// ************************************************************
// Library internal singleton wiring
// ************************************************************
DebugManager_ &DebugManager_::getInstance() {
  static DebugManager_ instance;
  return instance;
}

// This is a free function helper to allow the member function to
// be called from a callback.
void debugManagerLink(String message) {
  debugManager.debugMsg(message);
}

DebugManager_ &debugManager = debugManager.getInstance();