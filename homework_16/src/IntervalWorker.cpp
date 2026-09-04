#include "IntervalWorker.hpp"

IntervalWorker::IntervalWorker(std::chrono::milliseconds interval)
    : interval(interval)
{
}

auto IntervalWorker::run() -> void
{
    threadReady = true;

    auto nextWakeup = std::chrono::steady_clock::now();

    while (!stopRequested.load()) {
        nextWakeup += std::chrono::duration_cast<std::chrono::microseconds>(interval);  // / timeScale

        intervalTask();

        std::unique_lock<std::mutex> lock(mutex);

        cv.wait_until(lock, nextWakeup, [this]() { return stopRequested.load(); });
    }
}

auto IntervalWorker::start() -> void
{
    threadReady = false;
    stopRequested = false;
    workerThread = std::thread(&IntervalWorker::run, this);
}

auto IntervalWorker::isThreadReady() const -> bool
{
    return threadReady.load();
}

auto IntervalWorker::wait() -> void
{
    if (workerThread.joinable()) {
        workerThread.join();
    }
}

auto IntervalWorker::stop() -> void
{
    stopRequested = true;
    cv.notify_all();

    if (workerThread.joinable()) {
        workerThread.join();
    }
}

IntervalWorker::~IntervalWorker()
{
    stop();
}
