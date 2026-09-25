#pragma once

#include <Arduino.h>
#include <functional>

// ************************************************************
// Run work on the Arduino loop task.
//
// Web handlers (async_tcp task), WiFi events (arduino_events
// task) and UDP callbacks (async_udp task) all run on their own
// FreeRTOS tasks, possibly on the other core. If they touch the
// config, the OLED, I2C or SPIFFS while loop() is doing the same,
// Strings and buffers can be corrupted.
//
// runInLoop() hands a job to the loop task and blocks the caller
// until loop() has run it, so the job never overlaps with loop().
// Only use it from tasks that can safely wait (the web handlers):
// WiFi event handlers must not block, so they set flags instead.
// ************************************************************

// Call once, at the start of setup(), from the loop task
void loopTasksBegin();

// True if the caller is the Arduino loop task
bool onLoopTask();

// Run job on the loop task and wait for it to finish. Runs it
// directly if already on the loop task.
void runInLoop(const std::function<void()> &job);

// Run any waiting jobs and a requested restart. Call from loop().
void serviceLoopJobs();

// Restart the ESP32 from loop() after delayMs, so that a web
// response can go out first
void requestRestart(uint32_t delayMs);
