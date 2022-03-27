#pragma once

#include <Arduino.h>

typedef void (*DebugCallback) (String);

// Extended debug settings - these allow trace level debugging
#define TZM_EXTENDED_DEBUG_OFF
#define RTC_EXTENDED_DEBUG_OFF

class DebugManager_ {
  private:
    DebugManager_() = default; // Make constructor private

  public:
    static DebugManager_ &getInstance(); // Accessor for singleton instance

    DebugManager_(const DebugManager_ &) = delete; // no copying
    DebugManager_ &operator=(const DebugManager_ &) = delete;

  public:
    void begin();
    void debugMsg(String message);
    void debugMsgCont(String message);
    DebugCallback getDebugCallBack();

  private:
};

// free function link to the class function
extern void debugManagerLink(String message);

extern DebugManager_ &debugManager;