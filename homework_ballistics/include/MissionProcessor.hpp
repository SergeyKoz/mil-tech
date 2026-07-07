#pragma once

#include "IBallisticsSolver.hpp"
#include "IConfigLoader.hpp"
#include "common.hpp"

struct Target;
class ITargetsProvider;
class IConfigLoader;
class IBallisticsSolver;
class ISimulationExport;
class TargetSelector;
struct SelectedTarget;

struct SimStep {
    Coord pos;          // позиція дрона
    float direction;    // напрямок (рад)
    DroneStatus state;  // стан автомата (0-4)
    int targetIdx;      // індекс поточної цілі
    Coord dropPoint;    // точка скиду (куди летить дрон)
    Coord aimPoint;     // куди впаде бомба (якщо скинути зараз)
    Coord predictedTarget;
    float speed;
};

class MissionProcessor {
  public:
    MissionProcessor(IBallisticsSolver& solver,
                     IConfigLoader& configLoader,
                     ITargetsProvider& targetProvider,
                     ISimulationExport& simulationExport);
    void init();  //
    bool hasNext();
    void step();
    void reset();
    void changeSolver(IBallisticsSolver& solver);
    void dumpResults();
    ~MissionProcessor();

  private:
    static constexpr int MAX_STEPS = 5000;
    SimStep out[MAX_STEPS];
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
    inline void doAcceleration(SimStep& simStep, float acceleration, float time, float attackSpeed);
    inline void doDeceleration(SimStep& simStep, float acceleration, float time);
    inline void doMoving(SimStep& simStep, float time);
    inline void doTurning(SimStep& simStep, float turnAngle, float angleStep);
};
