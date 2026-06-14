#pragma once

#include <array>
#include <memory>
#include "interfaces/IBallisticsSolver.hpp"
#include "interfaces/IConfigLoader.hpp"
#include "interfaces/IDroneState.hpp"
#include "common.hpp"

class ITargetsProvider;
class IConfigLoader;
class IBallisticsSolver;
class ISimulationExport;
class TargetSelector;
struct SelectedTarget;

class MissionProcessor {
  public:
    static constexpr int MAX_STEPS = 10000;
    MissionProcessor(std::unique_ptr<IBallisticsSolver> solver,
                     std::unique_ptr<IConfigLoader> configLoader,
                     std::unique_ptr<ITargetsProvider> targetProvider,
                     std::unique_ptr<ISimulationExport> simulationExport);
    void init();
    bool hasNext();
    void step();
    void reset();
    void changeSolver(std::unique_ptr<IBallisticsSolver> solver);
    void dumpResults();
    ~MissionProcessor();

  private:
    std::array<SimStep, MAX_STEPS> out;
    DroneConfig droneConfig;
    SimStep simulationStep;
    std::unique_ptr<IBallisticsSolver> solver;
    std::unique_ptr<ITargetsProvider> targetProvider;
    std::unique_ptr<IConfigLoader> configLoader;
    std::unique_ptr<ISimulationExport> simulationExport;

    std::unique_ptr<IDroneState> state;
    std::unique_ptr<DroneContext> context;

    int currentStep;
    float currentTime;
    DropParameters dropParams;
    std::unique_ptr<TargetSelector> targetSelector;
    void calcSimulationStep(const SelectedTarget& selectedTarget);
    bool isTargetHit(SimStep& simStep);
};
