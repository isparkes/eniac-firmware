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
// recalculate "slow moving" parameters
// ************************************************************
void LEDManager_::recalculateVariables() {
  _backlightDim = (float) cc->backlightDimFactor / (float) 100;

#ifdef FEATURE_EXT_LEDS
  _underlightDim = (float) cc->extDimFactor / (float) 100;
#endif

  if (cc->hueOffset == 0) {
    _invertSecondLed = false;
    _hueOffset = 0;
  } else {
    _invertSecondLed = true;
    int offset = cc->hueOffset % 360;
    _hueOffset = (offset) / 360.0;
    if (offset != cc->hueOffset) cc->hueOffset = offset; 
  }

  int offset = cc->towerHueOffset % 360;
  _towerHueOffset = (offset) / 360.0;
  if (offset != cc->towerHueOffset) cc->towerHueOffset = offset; 
}

// ************************************************************
// Set the LDR dimming value
// ************************************************************
void LEDManager_::setLDRValue(unsigned int ldrValue)
{
  if (cc->useBLDim) {
    _ldrDimFactor = (float) (_ldrRange - ldrValue) / _ldrRange;
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
void LEDManager_::setPulseValue(unsigned int secsDelta)
{
  if (cc->useBLPulse) {
    // Calculate the brightness factor based on the "pulse"
    _pwmFactor = (float) secsDelta / (float) 1000.0;
  }
}

// ************************************************************
// Set blank status
// ************************************************************
void LEDManager_::setBlanked(boolean blanked)
{
  _blanked = blanked;
}

// ************************************************************
// Set back light LEDs to the same colour
// ************************************************************
void LEDManager_::setBacklightLEDs(byte red, byte green, byte blue) {
  for (int i = 0 ; i < NUM_BL_PIXELS ; i++) {
    setBacklightLED(i, red, green, blue, _invertSecondLed);
  }
}

// ************************************************************
// Set a single back light LEDs to a colour
// ************************************************************
void LEDManager_::setBacklightLED(byte index, byte red, byte green, byte blue, bool invert) {
  if ((invert) && isSecondLED(index)) {
    uint8_t inv_red = 0;
    uint8_t inv_green = 0;
    uint8_t inv_blue = 0;
    adjustRGB(red, green, blue, inv_red, inv_green, inv_blue, _hueOffset);
    ledRb[LED_ADDR[index]] = inv_red;
    ledGb[LED_ADDR[index]] = inv_green;
    ledBb[LED_ADDR[index]] = inv_blue;
  } else {
    ledRb[LED_ADDR[index]] = red;
    ledGb[LED_ADDR[index]] = green;
    ledBb[LED_ADDR[index]] = blue;
  }
}

// ************************************************************
// Set a single back light LEDs to a colour
// ************************************************************
void LEDManager_::setTowerLEDs(byte red, byte green, byte blue) {
  #ifdef FEATURE_SEP_LED
    uint8_t inv_red = 0;
    uint8_t inv_green = 0;
    uint8_t inv_blue = 0;
    adjustRGB(red, green, blue, inv_red, inv_green, inv_blue, _towerHueOffset);
    ledRb[4] = inv_red;
    ledGb[4] = inv_green;
    ledBb[4] = inv_blue;
    
    ledRb[9] = inv_red;
    ledGb[9] = inv_green;
    ledBb[9] = inv_blue;
  #endif
}

// ************************************************************
// Set under light LEDs to the same colour
// ************************************************************
void LEDManager_::setUnderlightLEDs(byte red, byte green, byte blue) {
  #ifdef FEATURE_EXT_LEDS
  for (int i = 0 ; i < DIGIT_COUNT ; i++) {
    setUnderlightLED(i, red, green, blue);
  }
  #endif
}

// ************************************************************
// Set under light LEDs to the same colour
// ************************************************************
void LEDManager_::setUnderlightLED(byte index, byte red, byte green, byte blue) {
  #ifdef FEATURE_EXT_LEDS
  ledRu[index] = red;
  ledGu[index] = green;
  ledBu[index] = blue;
  #endif
}

// ************************************************************
// Set day of week for the 'day of week' backlight mode
// ************************************************************
void LEDManager_::setDayOfWeek(byte dow) {
  _dow = dow-1;
}

// ************************************************************
// Put the led buffers out
// ************************************************************
void LEDManager_::outputLEDBuffer() {
  for (int i = 0 ; i < NUM_PIXELS_TOTAL - NUM_UL_PIXELS ; i++) {
#ifdef REVERSE_BL_OUTPUT
    RgbColor color(ledRb[NUM_BL_PIXELS - i - 1], ledGb[NUM_BL_PIXELS - i - 1], ledBb[NUM_BL_PIXELS - i - 1]);
#else
    RgbColor color(ledRb[i], ledGb[i], ledBb[i]);
#endif
    leds.SetPixelColor(i, color);
  }
  
  for (int i = 0 ; i < NUM_UL_PIXELS ; i++) {
#ifdef REVERSE_UL_OUTPUT
    RgbColor color(ledRu[DIGIT_COUNT - i - 1], ledGu[DIGIT_COUNT - i - 1], ledBu[DIGIT_COUNT - i - 1]);
#else
    RgbColor color(ledRu[i], ledGu[i], ledBu[i]);
#endif
    leds.SetPixelColor(i + NUM_BL_PIXELS, color);
  }
  leds.Show();
}

// ************************************************************
// Process the options and create a new buffer
// ************************************************************
void LEDManager_::processLedStatus() {
  // -------------------------------- Backlights / Underlights -------------------------------

  if (_blanked) {
    setBacklightLEDs( getLEDAdjustedBL(0),
                      getLEDAdjustedBL(0),
                      getLEDAdjustedBL(0));
    setUnderlightLEDs(getLEDAdjustedUL(0),
                      getLEDAdjustedUL(0),
                      getLEDAdjustedUL(0));
  } else {
    switch (cc->backlightMode) {
      case BACKLIGHT_FIXED: {
          setBacklightLEDs( getLEDAdjustedBL(rgb_backlight_curve[cc->redCnl]),
                            getLEDAdjustedBL(rgb_backlight_curve[cc->grnCnl]),
                            getLEDAdjustedBL(rgb_backlight_curve[cc->bluCnl]));
          setUnderlightLEDs(getLEDAdjustedUL(rgb_backlight_curve[cc->redCnl]),
                            getLEDAdjustedUL(rgb_backlight_curve[cc->grnCnl]),
                            getLEDAdjustedUL(rgb_backlight_curve[cc->bluCnl]));
          break;
        }
      case BACKLIGHT_CYCLE: {
          cycleColours3(colors);
          setBacklightLEDs( getLEDAdjustedBL(colors[0]),
                            getLEDAdjustedBL(colors[1]),
                            getLEDAdjustedBL(colors[2]));
          setUnderlightLEDs(getLEDAdjustedUL(colors[0]),
                            getLEDAdjustedUL(colors[1]),
                            getLEDAdjustedUL(colors[2]));
          break;
        }
      case BACKLIGHT_COLOUR_TIME: {
          if (!_syncColourTime) {
            for (byte i = 0 ; i < NUM_BL_PIXELS ; i++) {
              byte numVal = numberArray[i/2];
              setBacklightLED(i,
                              getLEDAdjustedBL(colourTimeR[numVal]),
                              getLEDAdjustedBL(colourTimeG[numVal]),
                              getLEDAdjustedBL(colourTimeB[numVal]),
                              _invertSecondLed);
              setUnderlightLED(i, 
                              getLEDAdjustedUL(colourTimeR[numVal]),
                              getLEDAdjustedUL(colourTimeG[numVal]),
                              getLEDAdjustedUL(colourTimeB[numVal]));
            }
          }
          break;
        }
      case BACKLIGHT_DAY_OF_WEEK: {
          setBacklightLEDs( getLEDAdjustedBL(dayOfWeekR[_dow]),
                            getLEDAdjustedBL(dayOfWeekG[_dow]),
                            getLEDAdjustedBL(dayOfWeekB[_dow]));
          setUnderlightLEDs(getLEDAdjustedUL(dayOfWeekR[_dow]),
                            getLEDAdjustedUL(dayOfWeekG[_dow]),
                            getLEDAdjustedUL(dayOfWeekB[_dow]));
          break;
        }
    }
    setTowerLEDs(   getLEDAdjustedUL(255),
                    getLEDAdjustedUL(0),
                    getLEDAdjustedUL(0));    
  }

  outputLEDBuffer();
}

// ************************************************************
// Process the options and create a new buffer
// ************************************************************
void LEDManager_::setTestValue(byte value) {
  // -------------------------------- Backlights / Underlights -------------------------------

  byte numVal = value%10;
  for (byte i = 0 ; i < NUM_BL_PIXELS ; i++) {
    setBacklightLED(i, 
                    testColoursR[numVal],
                    testColoursG[numVal],
                    testColoursB[numVal],
                    false);
    setUnderlightLED(i, 
                    testColoursR[numVal],
                    testColoursG[numVal],
                    testColoursB[numVal]);
  }

  setTowerLEDs(   getLEDAdjustedUL(255),
                  getLEDAdjustedUL(0),
                  getLEDAdjustedUL(0));

  outputLEDBuffer();
}

// ************************************************************
// output a PWM LED channel, adjusting for dimming, PWM
// and user back light brightness
// ************************************************************
byte LEDManager_::getLEDAdjustedBL(byte rawValue) {
  byte dimmedPWMVal;
  if (cc->useBLDim) {
    if (cc->useBLPulse) {
      dimmedPWMVal = (byte)(rawValue * _pwmFactor * _backlightDim * _ldrDimFactor);
    } else {
      dimmedPWMVal = (byte)(rawValue * _backlightDim * _ldrDimFactor);
    }
  } else {
    if (cc->useBLPulse) {
      dimmedPWMVal = (byte)(rawValue * _pwmFactor * _backlightDim);
    } else {
      dimmedPWMVal = (byte)(rawValue * _backlightDim);
    }
  }
  return dim_curve[dimmedPWMVal];
}

// ************************************************************
// output a PWM LED channel, adjusting for dimming, PWM
// and user under light brightness
// ************************************************************
byte LEDManager_::getLEDAdjustedUL(byte rawValue) {
  byte dimmedPWMVal;
  if (cc->useBLDim) {
    if (cc->useBLPulse) {
      dimmedPWMVal = (byte)(rawValue * _pwmFactor * _underlightDim * _ldrDimFactor);
    } else {
      dimmedPWMVal = (byte)(rawValue * _underlightDim * _ldrDimFactor);
    }
  } else {
    if (cc->useBLPulse) {
      dimmedPWMVal = (byte)(rawValue * _pwmFactor * _underlightDim);
    } else {
      dimmedPWMVal = (byte)(rawValue * _underlightDim);
    }
  }
  return dim_curve[dimmedPWMVal];
}

// ************************************************************
// Colour cycling 3: one colour dominates
// ************************************************************
void LEDManager_::cycleColours3(int colors[3]) {
  _cycleCount++;
  if (_cycleCount > cc->cycleSpeed) {
    _cycleCount = 0;

    if (changeSteps == 0) {
      changeSteps = random(256);
      currentColour = random(3);
    }

    changeSteps--;

    switch (currentColour) {
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
          changeSteps = 0;
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
          changeSteps = 0;
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
          changeSteps = 0;
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
      setBacklightLED(i, 0x1f, 0x1f, 0x1f, false);
      setUnderlightLED(i, 0x1f, 0x1f, 0x1f);
    } else if (i == stepNumber) {
      if (state == STATUS_RED) {
      setBacklightLED(i, 0xff, 0, 0, false);
      setUnderlightLED(i, 0xff, 0, 0);
      } else if (state == STATUS_YELLOW) {
      setBacklightLED(i, 0xff, 0x7f, 0x0f, false);
      setUnderlightLED(i, 0xff, 0x7f, 0x0f);
      } else if (state == STATUS_GREEN) {
      setBacklightLED(i, 0, 0xff, 0, false);
      setUnderlightLED(i, 0, 0xff, 0);
      } else if (state == STATUS_BLUE) {
      setBacklightLED(i, 0, 0, 0xff, false);
      setUnderlightLED(i, 0, 0, 0xff);
      }
    }
  }
  outputLEDBuffer();
}

// ************************************************************
// See if the led should be inverted
// ************************************************************
bool LEDManager_::isSecondLED(byte index) {
  return (index %2 == 1);
}

// ************************************************************
// Find the colour wheel inverse of the input RGB colour
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

LEDManager_ &ledManager = ledManager.getInstance();
