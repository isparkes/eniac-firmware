#pragma once

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
};

extern MyLib_ &MyLib;