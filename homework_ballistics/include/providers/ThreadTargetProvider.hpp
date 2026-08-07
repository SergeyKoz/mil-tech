#pragma once

#include <vector>
#include <fstream>
#include "IntervalWorker.hpp"
#include "interfaces/ITargetsProvider.hpp"

class ThreadTargetProvider : public IntervalWorker, public ITargetsProvider {
  public:
    ThreadTargetProvider(std::ifstream targetsFile, const DroneConfig& droneConfig);

    auto load() -> void override;
    auto getTargetsCount() -> int override;
    auto getTimeSteps() -> int override;
    auto getTarget(int index) -> Target* override;

    ~ThreadTargetProvider();

  private:
    std::mutex dataMutex;
    std::atomic<int> msSinceStart{0};

    std::ifstream targetsFile;
    int targetsCount;
    int timeSteps;
    int arrayTimeStep;
    std::vector<std::unique_ptr<Target>> targets;

    auto intervalTask() -> void override;
};