#pragma once

#include <FS.h>
#include <ArduinoJson.h>
#include "SPIFFS.h"
#include "DebugManager.h"
#include "Globals.h"
#include "utilities.h"

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
    bool testMountSpiffs();
    bool getSpiffsMounted();

    // These load/store the global objects, defined in globals.h
    bool getConfigFromSpiffs();
    void saveConfigToSpiffs();
    bool getStatsFromSpiffs();
    void saveStatsToSpiffs();

    String getZoneConfigSpiffs();

    int getZoneAreaCountFromSpiffs();
    String getZoneAreaFromSpiffs(int index);

    int getZoneLocationCountFromSpiffs(String location);
    String getZoneLocationFromSpiffs(String location, int index);
    String getLocationTZFromSpiffs(String location, int index);

    JsonObject& getConfigAsJsonObject();
  private:
    bool _spiffsMounted = false;
    // ArduinoJson 5 parses a char* in place: the parsed object points
    // into _zonesText, so the text must live as long as _cachedZonesObj
    DynamicJsonBuffer _jsonBuffer;
    std::unique_ptr<char[]> _zonesText;
    JsonObject* _cachedZonesObj = nullptr;

    void getZoneInfoFromSpiffs();
    std::unique_ptr<char[]> readFileToBuffer(File &file);

};

extern SpiffsStorage_ &spiffsStorage;