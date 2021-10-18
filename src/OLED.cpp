#include "OLED.h"

void OLED_::setUp()
{
  _display.reset(new Adafruit_SSD1306(128,64, &Wire, -1));
  _display->begin(SSD1306_SWITCHCAPVCC, 0x3C);
  _display->setTextSize(1);
  _display->setTextColor(WHITE, BLACK);
  _display->clearDisplay();
}

void OLED_::clearDisplay()
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

void OLED_::blankDisplay()
{
  _display->clearDisplay();
  _display->display();
}

void OLED_::outputDisplay()
{
  _display->display();
}

void OLED_::showScrollingMessage(String messageText)
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

void OLED_::showStatusLine()
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

void OLED_::setTimeString(String newTimeText)
{
  timeText = newTimeText;
  drawTimeInd();
  _display->display();
}

void OLED_::setWiFiStatus(bool newStatus)
{
  wifiStatus = newStatus;
  drawWiFiInd();
  _display->display();
}

void OLED_::setNTPStatus(bool newStatus)
{
  ntpStatus = newStatus;
  drawNTPInd();
  _display->display();
}

void OLED_::setPIRStatus(bool newStatus)
{
  pirStatus = newStatus;
  drawPIRInd();
  _display->display();
}

void OLED_::setPIRInstalled(bool newStatus)
{
  pirInstalled = newStatus;
  drawPIRInd();
  _display->display();
}

void OLED_::setBlankStatus(bool newStatus)
{
  blankStatus = newStatus;
  drawBlankInd();
  _display->display();
}

void OLED_::setXStatus(bool newStatus)
{
  xStatus = newStatus;
  drawXInd();
  _display->display();
}

void OLED_::setYStatus(bool newStatus)
{
  yStatus = newStatus;
  drawYInd();
  _display->display();
}

void OLED_::setZStatus(bool newStatus)
{
  zStatus = newStatus;
  drawZInd();
  _display->display();
}

void OLED_::setAMStatus(bool newStatus)
{
  ampm = newStatus;
  showStatusLine();
  _display->display();
}

void OLED_::drawWiFiInd() {
  _display->setCursor(WIFI_IND_X,STATUS_LINE_Y);
  if (wifiStatus) {
    _display->print("W");
  } else {
    _display->print("w");
  }
}

void OLED_::drawNTPInd() {
  _display->setCursor(NTP_IND_X,STATUS_LINE_Y);
  if (ntpStatus) {
    _display->print("N");
  } else {
    _display->print("n");
  }
  _display->setTextColor(WHITE, BLACK);
}

void OLED_::drawPIRInd() {
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

void OLED_::drawBlankInd() {
  _display->setCursor(BLANK_IND_X,STATUS_LINE_Y);
  if (blankStatus) {
    _display->print("B");
  } else {
    _display->print("b");
  }
}

void OLED_::drawXInd() {
  _display->setCursor(X_IND_X,STATUS_LINE_Y);
  if (xStatus) {
    _display->print("X");
  } else {
    _display->print("x");
  }
}

void OLED_::drawYInd() {
  _display->setCursor(Y_IND_X,STATUS_LINE_Y);
  if (yStatus) {
    _display->print("Y");
  } else {
    _display->print("y");
  }
}

void OLED_::drawZInd() {
  _display->setCursor(Z_IND_X,STATUS_LINE_Y);
  if (zStatus) {
    _display->print("Z");
  } else {
    _display->print("z");
  }
}

void OLED_::drawTimeInd() {
  _display->setCursor(TIME_IND_X,STATUS_LINE_Y);
  _display->print(timeText);
}

void OLED_::drawAMInd() {
  _display->setCursor(AM_IND_X,STATUS_LINE_Y);
  if (ampm) {
    _display->print("AM");
  } else {
    _display->print("PM");
  }
}

OLED_ &OLED_::getInstance() {
  static OLED_ instance;
  return instance;
}

OLED_ &oled = oled.getInstance();