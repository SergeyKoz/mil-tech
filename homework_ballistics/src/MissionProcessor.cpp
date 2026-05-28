#include "MissionProcessor.hpp"
#include "TargetSelector.hpp"
#include "interfaces/IConfigLoader.hpp"
#include "interfaces/ITargetsProvider.hpp"
#include "interfaces/ISimulationExport.hpp"
#include "interfaces/IBallisticsSolver.hpp"

MissionProcessor::MissionProcessor(IBallisticsSolver& solver,
                                   IConfigLoader& configLoader,
                                   ITargetsProvider& targetProvider,
                                   ISimulationExport& simulationExport)
    : out({})
    , simulationStep{}
    , solver(&solver)
    , targetProvider(&targetProvider)
    , configLoader(&configLoader)
    , simulationExport(&simulationExport)
    , acceleration(0.F)
    , angleStep(0.F)
    , currentStep(0)
    , currentTime(0.F)
    , dropParams{}
    , targetSelector(new TargetSelector(targetProvider)){};

auto MissionProcessor::init() -> void
{
    configLoader->load();
    droneConfig = configLoader->getConfig();
    targetProvider->load();
    targetSelector->init(droneConfig);

    currentStep = 0;
    currentTime = 0.F;

    angleStep = droneConfig.angleStep();
    acceleration = droneConfig.acceleration();
    dropParams = solver->calcDropParameters(droneConfig.ammo, droneConfig.attackSpeed, droneConfig.altitude);

    simulationStep = {.pos = droneConfig.startPos,    // позиція дрона
                      .direction = 0.F,               // напрямок (рад)
                      .state = STOPPED,               // стан автомата(0 - 4)
                      .targetIdx = -1,                // індекс поточної цілі
                      .dropPoint = {0.F, 0.F},        // точка скиду (куди летить дрон)
                      .aimPoint = {0.F, 0.F},         // куди впаде бомба (якщо скинути зараз)
                      .predictedTarget = {0.F, 0.F},  // прогнозована позиція цілі
                      .speed = 0.F};
};

auto MissionProcessor::hasNext() -> bool
{
    if (currentStep >= MAX_STEPS + 1) {
        return false;
    }

    if (currentStep > 0) {
        out.at(currentStep - 1) = simulationStep;

        if (isTargetHit(simulationStep)) {
            return false;
        }
    }

    return true;
};

auto MissionProcessor::step() -> void
{
    auto selectedTarget = targetSelector->selectTarget(currentTime, simulationStep, dropParams.distance);
    calcSimulationStep(selectedTarget);

    currentStep++;
    currentTime += 0.1F;
};

auto MissionProcessor::reset() -> void
{
    currentStep = 0;
    currentTime = 0.F;

    simulationStep = {.pos = droneConfig.startPos,    // позиція дрона
                      .direction = 0.F,               // напрямок (рад)
                      .state = STOPPED,               // стан автомата(0 - 4)
                      .targetIdx = -1,                // індекс поточної цілі
                      .dropPoint = {0.F, 0.F},        // точка скиду (куди летить дрон)
                      .aimPoint = {0.F, 0.F},         // куди впаде бомба (якщо скинути зараз)
                      .predictedTarget = {0.F, 0.F},  // прогнозована позиція цілі
                      .speed = 0.F};
};

auto MissionProcessor::changeSolver(IBallisticsSolver& solver) -> void
{
    this->solver = &solver;
}

auto MissionProcessor::calcSimulationStep(const SelectedTarget& selectedTarget) -> void
{
    simulationStep.targetIdx = selectedTarget.idx;

    // next coords
    Coord interpolatedPos = selectedTarget.target->interpolate(currentTime + droneConfig.simTimeStep, droneConfig.arrayTimeStep);
    Coord delta = interpolatedPos - selectedTarget.position;

    float vx = delta.x / droneConfig.simTimeStep;
    float vy = delta.y / droneConfig.simTimeStep;

    simulationStep.predictedTarget = {
        selectedTarget.position.x + vx * selectedTarget.timeToReachPosition,
        selectedTarget.position.y + vy * selectedTarget.timeToReachPosition,
    };

    float targetDirection = simulationStep.pos.direction(simulationStep.predictedTarget);
    float turnAngle = targetDirection - simulationStep.direction;
    bool isNeedTurnAngle = false;

    if (simulationStep.state == TURNING) {
        isNeedTurnAngle = std::fabs(turnAngle) > angleStep;
    }
    else {
        isNeedTurnAngle = std::fabs(turnAngle) > droneConfig.turnThreshold;
    }

    // need reentry
    float distanceToTarget = simulationStep.pos.distance(simulationStep.predictedTarget);
    float distanceToDropPoint = distanceToTarget - dropParams.distance;
    float reEntryPath = distanceToDropPoint < 0 ? -distanceToDropPoint : 0.F;

    if (!isNeedTurnAngle) {
        if (simulationStep.speed < droneConfig.attackSpeed) {
            float accelerationPath =
                (droneConfig.attackSpeed * droneConfig.attackSpeed - simulationStep.speed * simulationStep.speed) / (2 * acceleration);

            if (distanceToDropPoint - accelerationPath < 0) {
                reEntryPath += accelerationPath;
            }
        }
    }
    else {
        float stopingPath = 0.F;

        if (simulationStep.speed > 0) {
            stopingPath = (simulationStep.speed * simulationStep.speed) / (2 * acceleration);
        }

        if (distanceToDropPoint < stopingPath + droneConfig.accelerationPath) {
            reEntryPath = reEntryPath + stopingPath + droneConfig.accelerationPath;
        }
    }

    if (reEntryPath > 0) {
        // perform reentry maneur
        float reversDirection = simulationStep.predictedTarget.direction(simulationStep.pos);
        float turnAngle = reversDirection - simulationStep.direction;
        bool isReverseDirection = std::fabs(turnAngle) < droneConfig.turnThreshold;

        if (!isReverseDirection) {
            if (simulationStep.state == MOVING || simulationStep.state == ACCELERATING || simulationStep.state == DECELERATING) {
                // need stop
                simulationStep.state = DECELERATING;
                doDeceleration(simulationStep, acceleration, droneConfig.simTimeStep);
            }
            else if (simulationStep.state == STOPPED || simulationStep.state == TURNING) {
                if (!isReverseDirection) {
                    simulationStep.direction = turnAngle > 0 ? simulationStep.direction + angleStep : simulationStep.direction - angleStep;
                    simulationStep.state = TURNING;
                }
                else {
                    simulationStep.state = ACCELERATING;
                }
            }
        }
        else {
            // flying away
            if (simulationStep.state == TURNING || simulationStep.state == STOPPED || simulationStep.state == ACCELERATING) {
                simulationStep.state = ACCELERATING;
                doAcceleration(simulationStep, acceleration, droneConfig.simTimeStep, droneConfig.attackSpeed);
            }
            else if (simulationStep.state == MOVING) {
                doMoving(simulationStep, droneConfig.simTimeStep);
            }
        }
    }
    else {
        // perform entry maneur
        if (isNeedTurnAngle) {
            // perform turn maneur
            if (simulationStep.state == STOPPED || simulationStep.state == TURNING) {
                doTurning(simulationStep, turnAngle, angleStep);
            }
            else if (simulationStep.state == MOVING || simulationStep.state == ACCELERATING || simulationStep.state == DECELERATING) {
                simulationStep.state = DECELERATING;
                doDeceleration(simulationStep, acceleration, droneConfig.simTimeStep);
            }
        }
        else {
            if (simulationStep.state == STOPPED || simulationStep.state == TURNING || simulationStep.state == DECELERATING ||
                simulationStep.state == ACCELERATING) {
                simulationStep.state = ACCELERATING;
                doAcceleration(simulationStep, acceleration, droneConfig.simTimeStep, droneConfig.attackSpeed);
            }
            else if (simulationStep.state == MOVING) {
                doMoving(simulationStep, droneConfig.simTimeStep);
            }
        }
    }

    float targetDistance = simulationStep.pos.distance(interpolatedPos);
    simulationStep.dropPoint = simulationStep.pos.move(targetDistance - dropParams.distance, simulationStep.direction);
};

auto MissionProcessor::isTargetHit(SimStep& simStep) -> bool
{
    // check hitRadius
    Coord interpolatedPos = targetProvider->getTarget(simStep.targetIdx).interpolate(currentTime, droneConfig.arrayTimeStep);
    if (simStep.speed >= droneConfig.attackSpeed) {
        simStep.aimPoint = simStep.pos.move(dropParams.distance, simStep.direction);
        float DF = simStep.aimPoint.distance(interpolatedPos);

        if (DF <= droneConfig.hitRadius) {
            return true;
        }
    }
    else {
        simStep.aimPoint = {0.F, 0.F};
    }

    return false;
}

auto MissionProcessor::dumpResults() -> void
{
    simulationExport->dumpResults(currentStep, out);
}

MissionProcessor::~MissionProcessor()
{
    delete targetSelector;
};

auto MissionProcessor::doAcceleration(SimStep& simStep, float acceleration, float time, float attackSpeed) -> void
{
    float path = simStep.speed * time + 0.5F * acceleration * time * time;
    simStep.pos = simStep.pos.move(path, simStep.direction);

    simStep.speed += acceleration * time;

    if (simStep.speed >= attackSpeed) {
        simStep.speed = attackSpeed;
        simStep.state = MOVING;
    }
}

inline void MissionProcessor::doDeceleration(SimStep& simStep, float acceleration, float time)
{
    float path = simStep.speed * time - 0.5F * acceleration * time * time;
    simStep.pos = simStep.pos.move(path, simStep.direction);
    simStep.speed -= acceleration * time;

    if (simStep.speed <= 0.F) {
        simStep.state = STOPPED;
        simStep.speed = 0.F;
    }
}

inline void MissionProcessor::doMoving(SimStep& simStep, float time)
{
    float path = simStep.speed * time;
    simStep.pos = simStep.pos.move(path, simStep.direction);
}

inline void MissionProcessor::doTurning(SimStep& simStep, float turnAngle, float angleStep)
{
    simStep.direction = turnAngle > 0 ? simStep.direction + angleStep : simStep.direction - angleStep;
    simStep.state = std::fabs(turnAngle) >= angleStep ? TURNING : ACCELERATING;
}