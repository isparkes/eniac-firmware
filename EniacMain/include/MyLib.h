#pragma once

#include <Arduino.h>
#include "DebugManager.h"

typedef void (*DebugCallback) (String);

class MyLib_ {
  private:
    MyLib_() = default; // Make constructor private

  public:
    static MyLib_ &getInstance(); // Accessor for singleton instance

    MyLib_(const MyLib_ &) = delete; // no copying
    MyLib_ &operator=(const MyLib_ &) = delete;

  public:
    void begin();
    void doStuff();
  private:
};

extern MyLib_ &MyLib;