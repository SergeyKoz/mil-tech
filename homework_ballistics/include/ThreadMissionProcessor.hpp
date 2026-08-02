#pragma once

#include <map>
#include <functional>
#include <memory>
#include "ThreadDronePhysics.hpp"
#include "IntervalWorker.hpp"

class ITargetsProvider;
class IDroneState;
class IBallisticsSolver;
class ISimulationExport;
class TargetSelector;
class ThreadDronePhysics;
class ThreadDronePhysics;
struct DroneConfig;

class ThreadMissionProcessor : public IntervalWorker {
  public:
    static constexpr int MAX_STEPS = 10000;
    ThreadMissionProcessor(std::unique_ptr<IBallisticsSolver> solver,
                           const DroneConfig& droneConfig,
                           ThreadDronePhysics& dronePhysics,
                           ITargetsProvider& targetProvider,
                           std::unique_ptr<ISimulationExport> simulationExport);
    void init();

    auto dumpResults() -> void;
    ~ThreadMissionProcessor();

  private:
    std::mutex dataMutex;

    auto intervalTask() -> void override;

    DroneConfig droneConfig;
    ThreadDronePhysics* dronePhysics;
    std::unique_ptr<IBallisticsSolver> solver;
    std::unique_ptr<ISimulationExport> simulationExport;
    std::unique_ptr<TargetSelector> targetSelector;

    std::array<SimStep, MAX_STEPS> out;
    std::atomic<int> currentStep{0};
    float currentTime{0.F};
    DropParameters dropParams{};
    SimStep simulationStep{};
    std::unique_ptr<DroneContext> context;

    std::map<DroneStatus, std::function<std::unique_ptr<IDroneState>(TargetSelector&)>> states;

    auto calculateSimulationStep() -> SimStep;
    auto isTargetHit(SimStep& simStep) const -> bool;
};