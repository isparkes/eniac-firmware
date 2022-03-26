#include "MyLib.h"

void MyLib_::begin() {
  pinMode(LED_BUILTIN, OUTPUT);

  // blink the led a few times
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
  }

  digitalWrite(LED_BUILTIN, LOW);
}

void MyLib_::doStuff() {
  // blink the LED
  digitalWrite(LED_BUILTIN, HIGH);
  delay(200);
  digitalWrite(LED_BUILTIN, LOW);
  delay(800);
}

// ************************************************************
// Output a logging message to the debug output
// ************************************************************
void MyLib_::debugMsg(String message) {
  debugManager.debugMsg("[LIB]: " + message);
}

// ************************************************************
// Library internal singleton wiring
// ************************************************************
MyLib_ &MyLib_::getInstance() {
  static MyLib_ instance;
  return instance;
}

MyLib_ &MyLib = MyLib.getInstance();