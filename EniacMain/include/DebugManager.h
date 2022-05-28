#pragma once

#include <Arduino.h>
#include "Configuration.h"

typedef void (*DebugCallback) (String);

// Extended debug settings - these allow trace level debugging
#define OTM_EXTENDED_DEBUG_OFF
#define TZM_EXTENDED_DEBUG_OFF
#define RTC_EXTENDED_DEBUG_OFF
#define NTP_EXTENDED_DEBUG_OFF
#define LDR_EXTENDED_DEBUG_OFF
#define LED_EXTENDED_DEBUG_OFF
#define WFM_EXTENDED_DEBUG_OFF
#define SPF_EXTENDED_DEBUG_OFF

// Basic debug settings
#ifdef DEBUG_ON
#define debugMsgMain(message) debugManager.debugMsg("[LNC]", message);
#define debugMsgBlm(message) debugManager.debugMsg("[BLM]", message);
#define debugMsgGps(message) debugManager.debugMsg("[GPS]", message);
#define debugMsgLdr(message) debugManager.debugMsg("[LDR]", message);
#define debugMsgLed(message) debugManager.debugMsg("[LED]", message);
#define debugMsgNtp(message) debugManager.debugMsg("[NTP]", message);
#define debugMsgOtm(message) debugManager.debugMsg("[OTM]", message);
#define debugMsgRtc(message) debugManager.debugMsg("[RTC]", message);
#define debugMsgSlv(message) debugManager.debugMsg("[SLV]", message);
#define debugMsgSpf(message) debugManager.debugMsg("[SPF]", message);
#define debugMsgTzm(message) debugManager.debugMsg("[TZM]", message);
#define debugMsgWbm(message) debugManager.debugMsg("[WEB]", message);
#define debugMsgMnm(message) debugManager.debugMsg("[MNM]", message);
#define debugMsgUtl(message) debugManager.debugMsg("[UTL]", message);
#define debugMsgWfm(message) debugManager.debugMsg("[WFM]", message);
#else
#define debugMsgMain(message)
#define debugMsgBlm(message)
#define debugMsgGps(message)
#define debugMsgLdr(message)
#define debugMsgLed(message)
#define debugMsgNtp(message)
#define debugMsgOtm(message)
#define debugMsgRtc(message)
#define debugMsgSlv(message)
#define debugMsgSpf(message)
#define debugMsgTzm(message)
#define debugMsgWbm(message)
#define debugMsgMnm(message)
#define debugMsgUtl(message)
#define debugMsgWfm(message)
#endif

// Extended debug settings
#ifdef SPF_EXTENDED_DEBUG
#define debugMsgSpfX(message) debugManager.debugMsg("[UTL]", message);
#else
#define debugMsgSpfX(message)
#endif


class DebugManager_ {
  private:
    DebugManager_() = default; // Make constructor private

  public:
    static DebugManager_ &getInstance(); // Accessor for singleton instance

    DebugManager_(const DebugManager_ &) = delete; // no copying
    DebugManager_ &operator=(const DebugManager_ &) = delete;

  public:
    void setDebuggingOutput(bool newState);
    void debugMsg(String prefix, String message);
    void debugMsgCont(String message);
    void setDebugAutoOff(unsigned int seconds);
    void debugAutoOffCheck();
    bool isDebugOn();

    // Some components need to use a callback
    DebugCallback getDebugCallBack();

  private:
    bool _state = true;
    unsigned int _debugForSecs = 0;
};

// free function link to the class function
extern void debugManagerLink(String message);

extern DebugManager_ &debugManager;