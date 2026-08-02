#pragma once

#include <string>
#include <vector>
#include "IntervalWorker.hpp"
#include "interfaces/ITargetsProvider.hpp"

class ThreadTargetProvider : public IntervalWorker, public ITargetsProvider {
  public:
    ThreadTargetProvider(std::string jsonFilePath, const DroneConfig& droneConfig);

    auto load() -> void override;
    auto getTargetsCount() -> int override;
    auto getTimeSteps() -> int override;
    auto getTarget(int index) -> Target* override;

    ~ThreadTargetProvider();

  private:
    std::mutex dataMutex;
    std::atomic<int> msSinceStart{0};

    std::string jsonFilePath;
    int targetsCount;
    int timeSteps;
    int arrayTimeStep;
    std::vector<std::unique_ptr<Target>> targets;

    auto intervalTask() -> void override;
};