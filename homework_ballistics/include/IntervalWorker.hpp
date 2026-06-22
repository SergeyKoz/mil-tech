#pragma once

#include <thread>
#include <condition_variable>

class IntervalWorker {
  public:
    IntervalWorker(std::chrono::milliseconds interval, int timeScale);
    void init();
    void start();
    void wait();
    bool isThreadReady() const;
    void stop();
    virtual ~IntervalWorker();

  protected:
    std::chrono::milliseconds interval;
    std::atomic<bool> stopRequested{false};
    std::atomic<bool> threadReady{false};

  private:
    std::thread workerThread;
    std::mutex mutex;
    std::condition_variable cv;

    int timeScale;
    auto run() -> void;
    auto virtual intervalTask() -> void = 0;
};