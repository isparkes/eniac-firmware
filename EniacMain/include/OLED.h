#pragma once

#include <memory>
#include <Adafruit_GFX.h>       // v1.6.1
#include <Adafruit_SSD1306.h>   // v2.0.1

// ----------------------- Defines -----------------------

#define PIR_NOT_INSTALLED 0
#define PIR_NO_MOVEMENT   1
#define PIR_MOVEMENT      2

#define STATUS_LINE_Y 54
#define WIFI_IND_X     5
#define NTP_IND_X     12
#define G_IND_X       19
#define PIR_IND_X     26
#define BLANK_IND_X   33
#define Y_IND_X       40
#define Z_IND_X       47
#define TIME_IND_X    60
#define AM_IND_X     111

#define STATUS_BOX_X   0
#define STATUS_BOX_Y  52
#define STATUS_BOX_W 127
#define STATUS_BOX_H  12

// ----------------------------------------------------------------------------------------------------
// ------------------------------------------ OLED Component ------------------------------------------
// ----------------------------------------------------------------------------------------------------

class OLED_
{
  private:
    OLED_() = default; // Make constructor private

  public:
    static OLED_ &getInstance(); // Accessor for singleton instance

    OLED_(const OLED_ &) = delete; // no copying
    OLED_ &operator=(const OLED_ &) = delete;

  public:
    void setUp();
    void showMenuHeading(String menuText);
    void showMenuEntry(byte level, String menuText);
    void blankMenuEntry(byte level);
    void showStatusLine();
    void setWiFiStatus(bool newStatus);
    void setNTPStatus(bool newStatus);
    void setPIRStatus(bool newStatus);
    void setPIRInstalled(bool newStatus);
    void setBlankStatus(bool newStatus);
    void setGStatus(bool newStatus);
    void setYStatus(bool newStatus);
    void setZStatus(bool newStatus);
    void setAMStatus(bool newStatus);
    void clearDisplay();
    void blankDisplay();
    bool getBlanked();
    void outputDisplay();
    void clearScrollingMessage();
    void showScrollingMessage(String messageText);
    void setTimeString(String timeText);

    // Hacks for menu manager
    void setTextColor(uint16_t c);
    void setTextColor(uint16_t c, uint16_t bg);
    void setCursor(int16_t x, int16_t y);
    void setTextSize(uint8_t s);
    void println(const String &s);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    int16_t width();
    int16_t height();

  private:
    bool _blanked = false;
    bool _wifiStatus = false;
    bool _ntpStatus = false;
    byte pirStatus = false;
    byte pirInstalled = false;
    bool blankStatus = false;
    bool xStatus = false;
    bool yStatus = false;
    bool zStatus = false;
    bool ampm = false;
    String timeText = "xx:xx:xx";
    String bufferLines[6] = {"","","","","",""};
    byte bufferIdx = 0;
    std::unique_ptr<Adafruit_SSD1306> _display;

    void drawWiFiInd();
    void drawNTPInd();
    void drawPIRInd();
    void drawBlankInd();
    void drawGInd();
    void drawYInd();
    void drawZInd();
    void drawAMInd();
    void drawTimeInd();
};

extern OLED_ &oled;
