#pragma once

#include <array>
#include "interfaces/IBallisticsSolver.hpp"
#include "interfaces/IConfigLoader.hpp"
#include "common.hpp"

class ITargetsProvider;
class IConfigLoader;
class IBallisticsSolver;
class ISimulationExport;
class TargetSelector;
struct SelectedTarget;

class MissionProcessor {
  public:
    static constexpr int MAX_STEPS = 5000;
    MissionProcessor(IBallisticsSolver& solver,
                     IConfigLoader& configLoader,
                     ITargetsProvider& targetProvider,
                     ISimulationExport& simulationExport);
    void init();
    bool hasNext();
    void step();
    void reset();
    void changeSolver(IBallisticsSolver& solver);
    void dumpResults();
    ~MissionProcessor();

  private:
    std::array<SimStep, MAX_STEPS> out;
    DroneConfig droneConfig;
    SimStep simulationStep;
    IBallisticsSolver* solver;
    ITargetsProvider* targetProvider;
    IConfigLoader* configLoader;
    ISimulationExport* simulationExport;
    float acceleration;
    float angleStep;
    int currentStep;
    float currentTime;
    DropParameters dropParams;
    TargetSelector* targetSelector;
    void calcSimulationStep(const SelectedTarget& selectedTarget);
    bool isTargetHit(SimStep& simStep);
    static inline void doAcceleration(SimStep& simStep, float acceleration, float time, float attackSpeed);
    static inline void doDeceleration(SimStep& simStep, float acceleration, float time);
    static inline void doMoving(SimStep& simStep, float time);
    static inline void doTurning(SimStep& simStep, float turnAngle, float angleStep);
};
