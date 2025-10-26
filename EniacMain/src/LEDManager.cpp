#include "LEDManager.h"

#ifdef APA106
NeoPixelBus<NeoRgbFeature, Neo800KbpsMethod> leds(NUM_PIXELS_TOTAL, LED_DOUT);
#else
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> leds(NUM_PIXELS_TOTAL, LED_DOUT);
#endif

void LEDManager_::setUp()
{
  // Set up the LED output
  leds.Begin();
}

// ************************************************************
// called once per second
// ************************************************************
void LEDManager_::updateOncePerSecond() {
  recalculateVariables();
  #ifdef LED_EXTENDED_DEBUG
  debugMsgLed("_overallBLDimFactorPBL: " + String(_overallBLDimFactorPBL,2));
  debugMsgLed("_ledRb[0]: " + String(_ledRb[0]));
  #endif
}

// ************************************************************
// called once very loop (10mS or so)
// ************************************************************
void LEDManager_::updateOncePerLoop() {
  setLDRValue(ldrManager.getLDRValueBL());
  setPulseValue();
  processLedStatusLoop();
}

// ************************************************************
// recalculate "slow moving" parameters
// ************************************************************
void LEDManager_::recalculateVariables() {
  _backlightDim = (float) cc->backlightDimFactor / 100.0;

  #ifdef FEATURE_EXT_LEDS
  _underlightDim = (float) cc->extDimFactor / 100.0;
  #endif

  // cache the weekday - native range is 1..7, we need 0..6
  _dow = weekday() - 1;

  // Calculate the gradient per NP
  float _hueOffsetIncrement = ((cc->backlightGradient * 100) / (DIGIT_COUNT * 100.0));
  #ifdef LED_EXTENDED_DEBUG
  debugMsgLed("Gradient: " + String(cc->backlightGradient) + "/" + String(NUM_BL_PIXELS) + " -> " + String(_hueOffsetIncrement));
  debugMsgLed("Offset:   " + String(cc->hueOffset));
  #endif

  // Calculate the offset array - for the gradient we work in LED pairs
  // only the hue offset affects the individual LEDs in a pair
  // Do not use for ColourTime
  if (cc->backlightMode != BACKLIGHT_COLOUR_TIME) { 
    for (int index = 0 ; index < DIGIT_COUNT ; index++) {
      // we want each pair of LEDs to have the same increment
      int gradientOffset = (int) (_hueOffsetIncrement*index);
      _hueOffsetPerPixel[LED_ADDR[index*2]] = (gradientOffset % 360) / 360.0;
      _hueOffsetPerPixel[LED_ADDR[index*2+1]] = ((gradientOffset + cc->hueOffset) % 360) / 360.0;
    }
  } else {
    for (int index = 0 ; index < DIGIT_COUNT ; index++) {
      _hueOffsetPerPixel[LED_ADDR[index*2]] = 0.0;
      _hueOffsetPerPixel[LED_ADDR[index*2+1]] = (cc->hueOffset % 360) / 360.0;
    }
  }

  #ifdef FEATURE_SEP_LED
  _hueOffsetPerPixel[TOWER_1] = (cc->towerHueOffset % 360) / 360.0;
  _hueOffsetPerPixel[TOWER_2] = _hueOffsetPerPixel[TOWER_1];
  #endif

  #ifdef LED_EXTENDED_DEBUG
  debugMsgLed("Hues: " +  String(_hueOffsetPerPixel[0]) + "," +
                          String(_hueOffsetPerPixel[1]) + "," +
                          String(_hueOffsetPerPixel[2]) + "," +
                          String(_hueOffsetPerPixel[3]) + "," +
                          String(_hueOffsetPerPixel[4]) + "," +
                          String(_hueOffsetPerPixel[5]) + "," +
                          String(_hueOffsetPerPixel[6]) + "," +
                          String(_hueOffsetPerPixel[7]) + "," +
                          String(_hueOffsetPerPixel[8]) + "," +
                          String(_hueOffsetPerPixel[9]) + "," +
                          String(_hueOffsetPerPixel[10]) + "," +
                          String(_hueOffsetPerPixel[11]) + "," +
                          String(_hueOffsetPerPixel[12]) + "," +
                          String(_hueOffsetPerPixel[13]) + ",");
  #endif

  // Invert the sense of the cycle speed
  _cycleSpeed = CYCLE_SPEED_MAP[cc->cycleSpeed];

  // We don't need to set these each loop if we are blanked
  // so we set once here
  if (_blanked) {
    setBacklightLEDs(0, 0, 0);
#ifdef FEATURE_EXT_LEDS                      
    setUnderlightLEDs(0, 0, 0);
#endif
  }

  if (_towersBlanked) {
    setTowerLEDs(0, 0, 0);
  }
}

// ************************************************************
// Set the LDR dimming value : between 0.0 and 1.0
// ************************************************************
void LEDManager_::setLDRValue(unsigned int ldrValue)
{
  if (cc->useBLDim) {
    _ldrDimFactor = (float) (_ldrRange - ldrValue) / _ldrRange;
  } else {
    _ldrDimFactor = 1.0;
  }
}

// ************************************************************
// Set the LDR dimming range value
// ************************************************************
void LEDManager_::setLDRRange(unsigned int ldrRange)
{
    _ldrRange = (float) ldrRange;
}

// ************************************************************
// Set the pulse current value
// ************************************************************
void LEDManager_::setPulseValue()
{
  if (cc->useBLPulse) {
    // Calculate the brightness factor based on the "pulse"
    _pulseFactor = (float) secsDelta / (float) 1000.0;
  } else {
    _pulseFactor = 1.0;
  }
}

// ************************************************************
// Set back light LEDs to the same colour
// ************************************************************
void LEDManager_::setBacklightLEDs(byte red, byte green, byte blue) {
  for (int i = 0 ; i < NUM_BL_PIXELS ; i++) {
    setBacklightLED(i, red, green, blue);
  }
}

// ************************************************************
// Set back light LEDs to the same colour
// ************************************************************
void LEDManager_::setBacklightLEDsUnadjusted(byte red, byte green, byte blue) {
  for (int i = 0 ; i < NUM_BL_PIXELS ; i++) {
    setBacklightLED(i, red, green, blue);
  }
}

// ************************************************************
// Set a single back light LEDs to a colour
// ************************************************************
void LEDManager_::setBacklightLEDUnadjusted(byte index, byte red, byte green, byte blue) {
  _ledRb[LED_ADDR[index]] = red;
  _ledGb[LED_ADDR[index]] = green;
  _ledBb[LED_ADDR[index]] = blue;
}

// ************************************************************
// Set a single back light LEDs to a colour
// ************************************************************
void LEDManager_::setBacklightLED(byte index, byte red, byte green, byte blue) {
  uint8_t inv_red = 0;
  uint8_t inv_green = 0;
  uint8_t inv_blue = 0;
  adjustRGB(red, green, blue, inv_red, inv_green, inv_blue, _hueOffsetPerPixel[LED_ADDR[index]]);
  _ledRb[LED_ADDR[index]] = inv_red;
  _ledGb[LED_ADDR[index]] = inv_green;
  _ledBb[LED_ADDR[index]] = inv_blue;
}

// ************************************************************
// Set a single back light LEDs to a colour
// ************************************************************
void LEDManager_::setTowerLEDs(byte red, byte green, byte blue) {
  #ifdef FEATURE_SEP_LED
    uint8_t inv_red = 0;
    uint8_t inv_green = 0;
    uint8_t inv_blue = 0;
    adjustRGB(red, green, blue, inv_red, inv_green, inv_blue, _hueOffsetPerPixel[TOWER_1]);
    _ledRb[TOWER_1] = inv_red;
    _ledGb[TOWER_1] = inv_green;
    _ledBb[TOWER_1] = inv_blue;
    
    adjustRGB(red, green, blue, inv_red, inv_green, inv_blue, _hueOffsetPerPixel[TOWER_2]);
    _ledRb[TOWER_2] = inv_red;
    _ledGb[TOWER_2] = inv_green;
    _ledBb[TOWER_2] = inv_blue;
  #endif
}

#ifdef FEATURE_EXT_LEDS
// ************************************************************
// Set under light LEDs to the same colour
// ************************************************************
void LEDManager_::setUnderlightLEDs(byte red, byte green, byte blue) {
  for (int i = 0 ; i < DIGIT_COUNT ; i++) {
    setUnderlightLED(i, red, green, blue);
  }
}
#endif

#ifdef FEATURE_EXT_LEDS
// ************************************************************
// Set under light LEDs to the same colour
// ************************************************************
void LEDManager_::setUnderlightLED(byte index, byte red, byte green, byte blue) {
  _ledRu[index] = red;
  _ledGu[index] = green;
  _ledBu[index] = blue;
}
#endif

// ************************************************************
// Put the led buffers out
// ************************************************************
void LEDManager_::outputLEDBuffer() {
  for (int i = 0 ; i < NUM_BL_PIXELS + NUM_SEP_LED ; i++) {
#ifdef REVERSE_BL_OUTPUT
    int ledIdx = NUM_BL_PIXELS + NUM_SEP_LED - i - 1;
#else
    int ledIdx = i;
#endif
    RgbColor color(_ledRb[ledIdx], _ledGb[ledIdx], _ledBb[ledIdx]);
    leds.SetPixelColor(i, color);
  }

#ifdef FEATURE_EXT_LEDS                      
  for (int i = 0 ; i < NUM_UL_PIXELS ; i++) {
#ifdef REVERSE_UL_OUTPUT
    RgbColor color(_ledRu[NUM_UL_PIXELS - i], _ledGu[NUM_UL_PIXELS - i], _ledBu[NUM_UL_PIXELS - i]);
#else
    RgbColor color(_ledRu[i], _ledGu[i], _ledBu[i]);
#endif
    leds.SetPixelColor(i + NUM_BL_PIXELS + NUM_SEP_LED, color);
  }
#endif

  leds.Show();
}

// ************************************************************
// Process the options each loop
// ************************************************************
void LEDManager_::processLedStatusLoop() {

  // Recalculate the LED factors including pulse, fixed dim
  _overallBLDimFactorPB = _pulseFactor * _backlightDim;

  // Recalculate the LED factors including pulse, fixed dim and LDR
  _overallBLDimFactorPBL = _overallBLDimFactorPB * _ldrDimFactor;

#ifdef FEATURE_EXT_LEDS
  _overallULDimFactor = _pulseFactor * _underlightDim;
#endif

  // -------------------------------- Backlights / Underlights -------------------------------

  if (!_blanked) {
    byte tmpMode = cc->backlightMode;

    #ifdef FEATURE_TICKER
    if (_tickerOverride == up) {
      tmpMode = BACKLIGHT_UP;
//      debugMsgLed("Using override LED mode: " + String(_tickerOverride));
    } else if (_tickerOverride == down) {
      tmpMode = BACKLIGHT_DOWN;
//      debugMsgLed("Using override LED mode: " + String(_tickerOverride));
    } else if (_tickerOverride == unchanged) {
      tmpMode = BACKLIGHT_UNCHANGED;
//      debugMsgLed("Using override LED mode: " + String(_tickerOverride));
    }
    #endif

    switch (tmpMode) {
      case BACKLIGHT_FIXED: {
          setBacklightLEDs( getLEDAdjustedBL(rgb_backlight_curve[cc->redCnl]),
                            getLEDAdjustedBL(rgb_backlight_curve[cc->grnCnl]),
                            getLEDAdjustedBL(rgb_backlight_curve[cc->bluCnl]));
#ifdef FEATURE_EXT_LEDS                      
          setUnderlightLEDs(getLEDAdjustedUL(rgb_backlight_curve[cc->redCnl]),
                            getLEDAdjustedUL(rgb_backlight_curve[cc->grnCnl]),
                            getLEDAdjustedUL(rgb_backlight_curve[cc->bluCnl]));
#endif
          break;
        }
      case BACKLIGHT_CYCLE: {
          cycleColours3(_colors);
          setBacklightLEDs( getLEDAdjustedBL(_colors[0]),
                            getLEDAdjustedBL(_colors[1]),
                            getLEDAdjustedBL(_colors[2]));
#ifdef FEATURE_EXT_LEDS                      
          setUnderlightLEDs(getLEDAdjustedUL(_colors[0]),
                            getLEDAdjustedUL(_colors[1]),
                            getLEDAdjustedUL(_colors[2]));
#endif
          break;
        }
      case BACKLIGHT_COLOUR_TIME: {
          if (!_syncColourTime) {
            for (byte i = 0 ; i < NUM_BL_PIXELS ; i++) {
              byte numVal = outputManager.getCurrentDisplayDigitValue(i/2);
              setBacklightLED(i,
                              getLEDAdjustedBL(colourTimeR[numVal]),
                              getLEDAdjustedBL(colourTimeG[numVal]),
                              getLEDAdjustedBL(colourTimeB[numVal]));
#ifdef FEATURE_EXT_LEDS                      
              setUnderlightLED(numVal, 
                              getLEDAdjustedUL(colourTimeR[numVal]),
                              getLEDAdjustedUL(colourTimeG[numVal]),
                              getLEDAdjustedUL(colourTimeB[numVal]));
#endif
            }
          }
          break;
        }
      case BACKLIGHT_DAY_OF_WEEK: {
          setBacklightLEDs( getLEDAdjustedBL(dayOfWeekR[_dow]),
                            getLEDAdjustedBL(dayOfWeekG[_dow]),
                            getLEDAdjustedBL(dayOfWeekB[_dow]));
#ifdef FEATURE_EXT_LEDS                      
          setUnderlightLEDs(getLEDAdjustedUL(dayOfWeekR[_dow]),
                            getLEDAdjustedUL(dayOfWeekG[_dow]),
                            getLEDAdjustedUL(dayOfWeekB[_dow]));
#endif
          break;
        }
      #ifdef FEATURE_TICKER
      case BACKLIGHT_UP: {
          setTestValue(1);
          break;
        }
      case BACKLIGHT_DOWN: {
          setTestValue(3);
          break;
        }
      case BACKLIGHT_UNCHANGED: {
          setTestValue(9);
          break;
        }
      #endif
    }
  }

  // ----------------------------------------- Towers ----------------------------------------

  if (!_towersBlanked) {
    if(cc->useLDRSep) {
      setTowerLEDs(   getLEDAdjustedBL(255),
                      getLEDAdjustedBL(0),
                      getLEDAdjustedBL(0));
    } else {
      setTowerLEDs(   getLEDAdjustedBLNoLDR(255),
                      getLEDAdjustedBLNoLDR(0),
                      getLEDAdjustedBLNoLDR(0));
    }
  }

  outputLEDBuffer();
}

// ************************************************************
// Set a predefined test colour
// ************************************************************
void LEDManager_::setTestValue(byte value) {

  // -------------------------------- Backlights / Underlights -------------------------------

  byte numVal = value%10;
  for (byte i = 0 ; i < NUM_BL_PIXELS ; i++) {
    setBacklightLEDUnadjusted(i, 
                              testColoursR[numVal],
                              testColoursG[numVal],
                              testColoursB[numVal]);
    setBacklightLEDUnadjusted(i, 
                              testColoursR[numVal],
                              testColoursG[numVal],
                              testColoursB[numVal]);
  }

  // ----------------------------------------- Towers ----------------------------------------

  setTowerLEDs(   getLEDAdjustedBL(255),
                  getLEDAdjustedBL(0),
                  getLEDAdjustedBL(0));

  outputLEDBuffer();
}

// ************************************************************
// Get a PWM LED channel, adjusting for pulse and user back light brightness
// ************************************************************
byte LEDManager_::getLEDAdjustedBLNoLDR(byte rawValue) {
  byte dimmedPWMVal = (byte)(rawValue * _overallBLDimFactorPB);
  return dim_curve[dimmedPWMVal];
}

// ************************************************************
// Get a PWM LED channel, adjusting for dimming, Pulse
// and user back light brightness
// ************************************************************
byte LEDManager_::getLEDAdjustedBL(byte rawValue) {
  byte dimmedPWMVal = (byte)(rawValue * _overallBLDimFactorPBL);
  return dim_curve[dimmedPWMVal];
}

#ifdef FEATURE_EXT_LEDS
// ************************************************************
// Get a PWM LED channel, adjusting for dimming, PWM
// and user under light brightness
// ************************************************************
byte LEDManager_::getLEDAdjustedUL(byte rawValue) {
  byte dimmedPWMVal;
  dimmedPWMVal = (byte)(rawValue * _overallULDimFactor);
  return dim_curve[dimmedPWMVal];
}
#endif

// ************************************************************
// Get a PWM LED channel, without adjusting for amything - 
// Used in ticker display
// ************************************************************
byte LEDManager_::getLEDRawBL(byte rawValue) {
  return rawValue;
}

// ************************************************************
// Colour cycling 3: one colour dominates
// ************************************************************
void LEDManager_::cycleColours3(int colors[3]) {
  _cycleCount++;
  if (_cycleCount > _cycleSpeed) {
    _cycleCount = 0;

    if (_changeSteps == 0) {
      _changeSteps = random(256);
      _currentColour = random(3);
    }

    _changeSteps--;

    switch (_currentColour) {
      case 0:
        if (colors[0] < 255) {
          colors[0]++;
          if (colors[1] > 0) {
            colors[1]--;
          }
          if (colors[2] > 0) {
            colors[2]--;
          }
        } else {
          _changeSteps = 0;
        }
        break;
      case 1:
        if (colors[1] < 255) {
          colors[1]++;
          if (colors[0] > 0) {
            colors[0]--;
          }
          if (colors[2] > 0) {
            colors[2]--;
          }
        } else {
          _changeSteps = 0;
        }
        break;
      case 2:
        if (colors[2] < 255) {
          colors[2]++;
          if (colors[0] > 0) {
            colors[0]--;
          }
          if (colors[1] > 0) {
            colors[1]--;
          }
        } else {
          _changeSteps = 0;
        }
        break;
    }
  }
}

// ************************************************************
// Decides if we pause the colout time or not (false = pause)
// ************************************************************
void LEDManager_::setSyncColourTime(boolean value) {
  _syncColourTime = value;
}

// ************************************************************
// Set the diagnostic LED colour - progressively setting the
// LEDs to dignostic colours
// ************************************************************
void LEDManager_::setDiagnosticLED(byte stepNumber, byte state) {
  for (int i = 0 ; i < DIGIT_COUNT ; i++) {
    if (i > stepNumber) {
      setBacklightLED(i, 0x1f, 0x1f, 0x1f);
#ifdef FEATURE_EXT_LEDS
      setUnderlightLED(i, 0x1f, 0x1f, 0x1f);
#endif
    } else if (i == stepNumber) {
      if (state == STATUS_RED) {
      setBacklightLED(i, 0xff, 0, 0);
#ifdef FEATURE_EXT_LEDS
      setUnderlightLED(i, 0xff, 0, 0);
#endif
      } else if (state == STATUS_YELLOW) {
      setBacklightLED(i, 0xff, 0x7f, 0x0f);
#ifdef FEATURE_EXT_LEDS
      setUnderlightLED(i, 0xff, 0x7f, 0x0f);
#endif
      } else if (state == STATUS_GREEN) {
      setBacklightLED(i, 0, 0xff, 0);
#ifdef FEATURE_EXT_LEDS
      setUnderlightLED(i, 0, 0xff, 0);
#endif
      } else if (state == STATUS_BLUE) {
      setBacklightLED(i, 0, 0, 0xff);
#ifdef FEATURE_EXT_LEDS
      setUnderlightLED(i, 0, 0, 0xff);
#endif
      }
    }
  }
  outputLEDBuffer();
}

// ************************************************************
// External trigger for changing tube led blanking
// ************************************************************
void LEDManager_::setLEDBlanking(boolean newStatus) {
  _blanked = newStatus;
}

// ************************************************************
// External trigger for changing tube led blanking
// ************************************************************
void LEDManager_::setTowerBlanking(boolean newStatus) {
  _towersBlanked = newStatus;
}

#ifdef FEATURE_TICKER
// ************************************************************
// Allow temporary override of the normal program
// ************************************************************
void LEDManager_::setTickerOverrideValue(quote_direction_t newValue) {
  _tickerOverride = newValue;
}
#endif

// ************************************************************
// Find the colour wheel offset of the input RGB colour
// ************************************************************
void LEDManager_::adjustRGB(uint8_t red, uint8_t green, uint8_t blue, uint8_t& inv_red, uint8_t& inv_green, uint8_t& inv_blue, double hueOffset) {
  double hue, saturation, value;
  
	auto rd = static_cast<double>(red) / 255;
	auto gd = static_cast<double>(green) / 255;
	auto bd = static_cast<double>(blue) / 255;
	auto max_val = max(rd, max(gd, bd));
  auto min_val = min(rd, min(gd, bd));
	 
	value = max_val;

	auto d = max_val - min_val;
	saturation = max_val == 0 ? 0 : d / max_val;

	hue = 0;
	if (max_val != min_val)
	{
		if (max_val == rd)
		{
			hue = (gd - bd) / d + (gd < bd ? 6 : 0);
		}
		else if (max_val == gd)
		{
			hue = (bd - rd) / d + 2;
		}
		else if (max_val == bd)
		{
			hue = (rd - gd) / d + 4;
		}
		hue /= 6;
	}

  // move the hue round
  hue += hueOffset;
  if (hue >= 1.0) {hue-=1.0;}

  // Serial.println("Hue: " + String(hue));

	double r = 0.0, g = 0.0, b = 0.0;

	auto i = static_cast<int>(hue * 6);
	auto f = hue * 6 - i;
	auto p = value * (1 - saturation);
	auto q = value * (1 - f * saturation);
	auto t = value * (1 - (1 - f) * saturation);

	switch (i % 6)
	{
	case 0: r = value , g = t , b = p;
		break;
	case 1: r = q , g = value , b = p;
		break;
	case 2: r = p , g = value , b = t;
		break;
	case 3: r = p , g = q , b = value;
		break;
	case 4: r = t , g = p , b = value;
		break;
	case 5: r = value , g = p , b = q;
		break;
	}

	inv_red = static_cast<uint8_t>(r * 255);
	inv_green = static_cast<uint8_t>(g * 255);
	inv_blue = static_cast<uint8_t>(b * 255);
}

LEDManager_ &LEDManager_::getInstance() {
  static LEDManager_ instance;
  return instance;
}

// ************************************************************
// Create singleton instance
// ************************************************************
LEDManager_ &ledManager = ledManager.getInstance();
