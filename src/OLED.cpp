#include "OLED.h"

void OLED::setUp()
{
  _display.reset(new Adafruit_SSD1306(128,64, &Wire, -1));
  _display->begin(SSD1306_SWITCHCAPVCC, 0x3C);
  _display->setTextSize(1);
  _display->setTextColor(WHITE, BLACK);
  _display->clearDisplay();
}

void OLED::clearDisplay()
{
  _display->clearDisplay();
  showStatusLine();
  _display->setCursor(0,0);
  bufferIdx = 0;
  for (int i = 5 ; i > 0 ; i--) {
    bufferLines[i] = "";
  }
  showStatusLine();
}

void OLED::outputDisplay()
{
  _display->display();
}

void OLED::showScrollingMessage(String messageText)
{
  String formattedString = messageText + "                   ";
  formattedString = formattedString.substring(0,20);
  if (bufferIdx < 6) {
    bufferLines[bufferIdx] = formattedString;
    bufferIdx++;
  } else {
    for (int i = 1 ; i < 6 ; i++) {
      bufferLines[i-1] = bufferLines[i];
    }
    bufferLines[5] = formattedString;
  }

  _display->setCursor(0,0);
  for (int i = 0 ; i < 6 ; i++) {
    _display->println(bufferLines[i]);
  }
  _display->display();
}

void OLED::showStatusLine()
{
  _display->drawRect(STATUS_BOX_X, STATUS_BOX_Y, STATUS_BOX_W, STATUS_BOX_H, WHITE);
  drawWiFiInd();
  drawNTPInd();
  drawPIRInd();
  drawBlankInd();
  drawXInd();
  drawYInd();
  drawZInd();
  drawTimeInd();
  drawAMInd();
  _display->display();
}

void OLED::setTimeString(String newTimeText)
{
  timeText = newTimeText;
  drawTimeInd();
  _display->display();
}

void OLED::setWiFiStatus(bool newStatus)
{
  wifiStatus = newStatus;
  drawWiFiInd();
  _display->display();
}

void OLED::setNTPStatus(bool newStatus)
{
  ntpStatus = newStatus;
  drawNTPInd();
  _display->display();
}

void OLED::setPIRStatus(bool newStatus)
{
  pirStatus = newStatus;
  drawPIRInd();
  _display->display();
}

void OLED::setPIRInstalled(bool newStatus)
{
  pirInstalled = newStatus;
  drawPIRInd();
  _display->display();
}

void OLED::setBlankStatus(bool newStatus)
{
  blankStatus = newStatus;
  drawBlankInd();
  _display->display();
}

void OLED::setAMStatus(bool newStatus)
{
  ampm = newStatus;
  showStatusLine();
  _display->display();
}

void OLED::drawWiFiInd() {
  _display->setCursor(WIFI_IND_X,STATUS_LINE_Y);
  if (wifiStatus) {
    _display->print("W");
  } else {
    _display->print("w");
  }
}

void OLED::drawNTPInd() {
  _display->setCursor(NTP_IND_X,STATUS_LINE_Y);
  if (ntpStatus) {
    _display->print("N");
  } else {
    _display->print("n");
  }
  _display->setTextColor(WHITE, BLACK);
}

void OLED::drawPIRInd() {
  _display->setCursor(PIR_IND_X,STATUS_LINE_Y);
  if (pirInstalled) {
    if (pirStatus) {
      _display->print("P");
    } else {
      _display->print("p");
    }
  } else {
    _display->print("-");
  }
}

void OLED::drawBlankInd() {
  _display->setCursor(BLANK_IND_X,STATUS_LINE_Y);
  if (blankStatus) {
    _display->print("B");
  } else {
    _display->print("b");
  }
}

void OLED::drawXInd() {
  _display->setCursor(X_IND_X,STATUS_LINE_Y);
  if (xStatus) {
    _display->print("X");
  } else {
    _display->print("x");
  }
}

void OLED::drawYInd() {
  _display->setCursor(Y_IND_X,STATUS_LINE_Y);
  if (yStatus) {
    _display->print("Y");
  } else {
    _display->print("y");
  }
}

void OLED::drawZInd() {
  _display->setCursor(Z_IND_X,STATUS_LINE_Y);
  if (zStatus) {
    _display->print("Z");
  } else {
    _display->print("z");
  }
}

void OLED::drawTimeInd() {
  _display->setCursor(TIME_IND_X,STATUS_LINE_Y);
  _display->print(timeText);
}

void OLED::drawAMInd() {
  _display->setCursor(AM_IND_X,STATUS_LINE_Y);
  if (ampm) {
    _display->print("AM");
  } else {
    _display->print("PM");
  }
}
