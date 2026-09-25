#include "LoopTasks.h"

// A job waiting for the loop task. It lives on the caller's stack:
// the caller blocks until the job is done, so it stays valid.
struct LoopJob {
  const std::function<void()> *fn;
  SemaphoreHandle_t done;
};

#define LOOP_JOB_QUEUE_SIZE 8

static TaskHandle_t _loopTask = nullptr;
static QueueHandle_t _jobQueue = nullptr;

static volatile bool _restartRequested = false;
static volatile uint32_t _restartAtMillis = 0;

// ************************************************************
// Remember which task is the loop task and create the job queue
// ************************************************************
void loopTasksBegin() {
  _loopTask = xTaskGetCurrentTaskHandle();
  _jobQueue = xQueueCreate(LOOP_JOB_QUEUE_SIZE, sizeof(LoopJob *));
}

// ************************************************************
// Are we running on the loop task?
// ************************************************************
bool onLoopTask() {
  return xTaskGetCurrentTaskHandle() == _loopTask;
}

// ************************************************************
// Hand a job to the loop task and wait until it has been run
// ************************************************************
void runInLoop(const std::function<void()> &job) {
  if ((_jobQueue == nullptr) || onLoopTask()) {
    job();
    return;
  }

  StaticSemaphore_t doneBuffer;
  LoopJob loopJob;
  loopJob.fn = &job;
  loopJob.done = xSemaphoreCreateBinaryStatic(&doneBuffer);

  LoopJob *jobPtr = &loopJob;
  xQueueSend(_jobQueue, &jobPtr, portMAX_DELAY);

  // No timeout: the job points at this stack frame, so we must not
  // return before loop() has run it. If loop() stops, the loop task
  // watchdog restarts the clock anyway.
  xSemaphoreTake(loopJob.done, portMAX_DELAY);
  vSemaphoreDelete(loopJob.done);
}

// ************************************************************
// Run the waiting jobs, and restart if one was asked for
// ************************************************************
void serviceLoopJobs() {
  if (_jobQueue != nullptr) {
    LoopJob *jobPtr;
    while (xQueueReceive(_jobQueue, &jobPtr, 0) == pdTRUE) {
      (*jobPtr->fn)();
      xSemaphoreGive(jobPtr->done);
    }
  }

  if (_restartRequested && ((int32_t)(millis() - _restartAtMillis) >= 0)) {
    ESP.restart();
  }
}

// ************************************************************
// Ask loop() to restart the ESP32 after a short delay
// ************************************************************
void requestRestart(uint32_t delayMs) {
  _restartAtMillis = millis() + delayMs;
  _restartRequested = true;
}
