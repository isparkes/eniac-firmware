#pragma once

#include "Arduino.h"
#include "Globals.h"
#include "SpiffsStorage.h"
#include "DebugManager.h"

// -------------------------------------------------------------------------------
// LDR Manager deals with the light sensor. Has the following functions:
// 1) Set the blanking pin PWM value
// 2) read and smooth the raw LDR readings
// 3) Manages the PWM setting of the tube output
// -------------------------------------------------------------------------------
#define USE_LDR_DEFAULT       true

#define LDR_VALUE_MAX         4095 // The maximum LDR value - based on 12-bit ADC

#define DIM_DEFAULT           20   // The default minimum dim %
#define DIM_MIN               1    // The minimum dim %
#define DIM_MAX               100  // The maximum dim %

#define SENSOR_SENSIT_MIN     100  // Sensor Sensitivity
#define SENSOR_SENSIT_MAX     400
#define SENSOR_SENSIT_DEFAULT 200

#define SENSOR_THRSH_MIN      0    // Bright is when we have LDR value = 0, when we read less than this value, we have full brightness
#define SENSOR_THRSH_MAX      500
#define SENSOR_THRSH_DEFAULT  50

#define SENSOR_SMOOTH_READINGS_MIN     1
#define SENSOR_SMOOTH_READINGS_MAX     255
#define SENSOR_SMOOTH_READINGS_DEFAULT 100  // Speed at which the brighness adapts to changes

class LDRManager_
{
  private:
    LDRManager_() = default; // Make constructor private

  public:
    static LDRManager_ &getInstance(); // Accessor for singleton instance

    LDRManager_(const LDRManager_ &) = delete; // no copying
    LDRManager_ &operator=(const LDRManager_ &) = delete;

  public:
    void setUp();

    // Read the LDR and set the internal varaibles from it
    void  processLDRValue();

    // Get the smoothed values. The BL values are bound to different max and min
    // and do not have ACP
    int   getLDRValueTube();
    float getLDRValueTubePct();
    int   getLDRValueBL();
    float getLDRValueBLPct();

    int   getRawLDRValue();

    bool  isMinDim();
    bool  isMaxDim();

    void  setLDRValueToMin(bool newState);
    bool  getLDRValueSetToMin();
    void  setLDRValueToMax(bool newState);
    void  setLDRValueToMaxACP(bool newState);
    void  setBlankingDim(bool newState);
    
    bool  getIsFixedLDRValue();

    void updateOncePerLoop();
    void updateOncePerSecond();
  private:
    double _sensorFactor = (double)SENSOR_SENSIT_DEFAULT / 100.0;

    int _rawLDRValue = 0;

    // Tube Values
    double _sensorLDRSmoothedTube = 0;
    double _sensorLDRSmoothedTubeACP = 0;
    int   _ldrValueTube = 0;

    // Backlight Values
    double _sensorLDRSmoothedBL = 0;
    int   _ldrValueBL = 0;

    bool  _isMinDim;
    bool  _isMaxDim;
    bool  _setMinDim;
    bool  _setMaxDim;
    bool  _setMaxDimACP;
    bool  _blankingDim = false;

    int   _minDimTube;
    int   _maxDimTube;
    int   _setDimTube;

    int   _minDimBL;
    int   _maxDimBL;
    int   _setDimBL;

    double _offset;
    double _factor;

    const int LDRPWMChannel = 0;

    void setUpPWM();
    void recalculateVariables();
};

extern LDRManager_ &ldrManager;
