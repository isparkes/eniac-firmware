#pragma once

// TZ manager deals with the application of the Time Zone and DST to UTC

class TZManager_ {
  private:
    TZManager_() = default; // Make constructor private
    unsigned long _UTCoffset;

  public:
    static TZManager_ &getInstance(); // Accessor for singleton instance

    TZManager_(const TZManager_ &) = delete; // no copying
    TZManager_ &operator=(const TZManager_ &) = delete;

  public:
    void begin();
    void calculateCurrentOffset(int year, int mon, int day, int hour, int min, int sec);
    int  getCurrentUTCOffset();
};

extern TZManager_ &tzManager;