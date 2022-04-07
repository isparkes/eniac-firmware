#pragma once

#include <FS.h>
#include <ArduinoJson.h>
#include "SPIFFS.h"
#include "DebugManager.h"
#include "Defs.h"
#include "Globals.h"

// ----------------------------------------------------------------------------------------------------
// ------------------------------------- SPIFFS Clock Component ---------------------------------------
// ----------------------------------------------------------------------------------------------------

class SpiffsStorage_
{
  private:
    SpiffsStorage_() = default; // Make constructor private

  public:
    static SpiffsStorage_ &getInstance(); // Accessor for singleton instance

    SpiffsStorage_(const SpiffsStorage_ &) = delete; // no copying
    SpiffsStorage_ &operator=(const SpiffsStorage_ &) = delete;

  public:
    void setDebugOutput(bool newDebug);

    bool testMountSpiffs();
    bool getSpiffsMounted();

    // These load/store the global objects, defined in globals.h
    bool getConfigFromSpiffs();
    void saveConfigToSpiffs();
    bool getStatsFromSpiffs();
    void saveStatsToSpiffs();

    JsonObject& getConfigAsJsonObject();
  private:
    bool _spiffsMounted = false;
};

extern SpiffsStorage_ &spiffsStorage;