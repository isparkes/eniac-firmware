#pragma once

#include <Arduino.h>

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

    void setDebugOutput(bool newDebug);
    
    // callbacks
    void setDebugCallback(DebugCallback dbcb);
  private:
    DebugCallback _dbcb;
    bool _debug = false;

    void debugMsg(String message);                        // print a debug message to the callback
};

extern MyLib_ &MyLib;