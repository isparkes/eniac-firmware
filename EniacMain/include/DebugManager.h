#pragma once

#include <Arduino.h>

typedef void (*DebugCallback) (String);

// Basic debug settings
#ifdef DEBUG_ON
#define debugMsgMain(message) debugManager.debugMsg("[LNC]", message);
#define debugMsgBlm(message) debugManager.debugMsg("[BLM]", message);
#define debugMsgGps(message) debugManager.debugMsg("[GPS]", message);
#define debugMsgLdr(message) debugManager.debugMsg("[LDR]", message);
#define debugMsgNtp(message) debugManager.debugMsg("[NTP]", message);
#define debugMsgOtm(message) debugManager.debugMsg("[OTM]", message);
#define debugMsgRtc(message) debugManager.debugMsg("[RTC]", message);
#define debugMsgSlv(message) debugManager.debugMsg("[SLV]", message);
#define debugMsgSpf(message) debugManager.debugMsg("[SPF]", message);
#define debugMsgTzm(message) debugManager.debugMsg("[TZM]", message);
#define debugMsgWbm(message) debugManager.debugMsg("[WEB]", message);
#define debugMsgMnm(message) debugManager.debugMsg("[MNM]", message);
#define debugMsgUtl(message) debugManager.debugMsg("[UTL]", message);
#else
#define debugMsgMain(message)
#define debugMsgBlm(message)
#define debugMsgGps(message)
#define debugMsgLdr(message)
#define debugMsgNtp(message)
#define debugMsgOtm(message)
#define debugMsgRtc(message)
#define debugMsgSlv(message)
#define debugMsgSpf(message)
#define debugMsgTzm(message)
#define debugMsgWbm(message)
#define debugMsgMnm(message)
#define debugMsgUtl(message)
#endif

// Extended debug settings - these allow trace level debugging
#define TZM_EXTENDED_DEBUG_ON
#define RTC_EXTENDED_DEBUG_ON
#define NTP_EXTENDED_DEBUG_ON

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
    void debugMsg(String prefix, String message);
    void debugMsgCont(String message);
    DebugCallback getDebugCallBack();

  private:
};

// free function link to the class function
extern void debugManagerLink(String message);

extern DebugManager_ &debugManager;