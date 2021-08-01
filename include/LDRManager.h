#ifndef ldrmanager_h
#define ldrmanager_h

#include "defs.h"
#include "Arduino.h"
#include "SpiffsStorage.h"

// -------------------------------------------------------------------------------
#define LDR_VALUE_MAX         4095 // The maximum LDR value - based on 12-bit ADC

#define MIN_DIM_DEFAULT       20   // The default minimum dim count
#define MIN_DIM_MIN           0    // The minimum dim count
#define MIN_DIM_MAX           500  // The maximum dim count

#define SENSOR_SENSIT_MIN     100  // Sensor Sensitivity
#define SENSOR_SENSIT_MAX     400
#define SENSOR_SENSIT_DEFAULT 200

#define SENSOR_THRSH_MIN      0    // Bright is when we have LDR value = 0, when we read less than this value, we have full brightness
#define SENSOR_THRSH_MAX      500
#define SENSOR_THRSH_DEFAULT  50

#define SENSOR_SMOOTH_READINGS_MIN     1
#define SENSOR_SMOOTH_READINGS_MAX     255
#define SENSOR_SMOOTH_READINGS_DEFAULT 100  // Speed at which the brighness adapts to changes

class LDRManager
{
  public:
    void setUp();

    void getDimmingFromLDR();
    int getLDRValue();

    void setDebugOutput(bool newDebug);
    
    // callbacks
    void setDebugCallback(DebugCallback dbcb);

    // Shared config  
    void setConfigObject(spiffs_config_t* ccPtr);
  private:
    double sensorLDRSmoothed = 0;
    double sensorFactor = (double)SENSOR_SENSIT_DEFAULT / 100.0;
    int sensorSmoothCountLDR = SENSOR_SMOOTH_READINGS_DEFAULT;
    int _ldrValue = 0;
    
    DebugCallback _dbcb;
    bool _debug = false;

    spiffs_config_t* _cc;

    void debugMsg(String message);                        // print a debug message to the callback
};

// ----------------- Exported Variables ------------------

static LDRManager ldrManager;

#endif
