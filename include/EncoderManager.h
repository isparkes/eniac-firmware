#pragma once

#include <Arduino.h>
#include <ESP32Encoder.h>

class EncoderManager_ {
  private:
    EncoderManager_() = default; // Make constructor private

  public:
    static EncoderManager_ &getInstance(); // Accessor for singleton instance

    EncoderManager_(const EncoderManager_ &) = delete; // no copying
    EncoderManager_ &operator=(const EncoderManager_ &) = delete;

  public:
    void setup();
    int getCount();
    bool getButtonState();
    bool isAttached();
    void clearCount();

  private:
    ESP32Encoder _encoder;
};

extern EncoderManager_ &encoderManager;