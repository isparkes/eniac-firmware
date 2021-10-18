#pragma once

typedef struct {
  bool bl1;
  bool bl2;
  bool bl3;
  bool bl4;
  bool bl5;
  bool bl6;
} blinkelights_t;

class BlinkenlightsManager_ {
  private:
    BlinkenlightsManager_() = default; // Make constructor private

  public:
    static BlinkenlightsManager_ &getInstance(); // Accessor for singleton instance

    BlinkenlightsManager_(const BlinkenlightsManager_ &) = delete; // no copying
    BlinkenlightsManager_ &operator=(const BlinkenlightsManager_ &) = delete;

  public:
    void begin();
    void setBlinkenlightsStatus(blinkelights_t *bl);
    void setBlinkenlightsChase(blinkelights_t *bl);
  private:
};

extern BlinkenlightsManager_ &blinkenlightsManager;