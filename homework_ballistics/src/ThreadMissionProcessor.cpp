#include "ThreadMissionProcessor.hpp"
#include "TargetSelector.hpp"
#include "Target.hpp"
#include "interfaces/ISimulationExport.hpp"
#include "interfaces/IBallisticsSolver.hpp"
#include "states/AcceleratingState.hpp"
#include "states/DeceleratingState.hpp"
#include "states/MovingState.hpp"
#include "states/StoppedState.hpp"
#include "common.hpp"
#include "states/TurningState.hpp"

ThreadMissionProcessor::ThreadMissionProcessor(std::unique_ptr<IBallisticsSolver> solver,
                                               const DroneConfig& droneConfig,
                                               ThreadDronePhysics& dronePhysics,
                                               ITargetsProvider& targetProvider,
                                               std::unique_ptr<ISimulationExport> simulationExport)
    : IntervalWorker(std::chrono::milliseconds(static_cast<int>(std::round(droneConfig.simTimeStep * 1000))),
                     static_cast<int>(droneConfig.timeScale))
    , droneConfig(droneConfig)
    , dronePhysics(&dronePhysics)
    , solver(std::move(solver))
    , simulationExport(std::move(simulationExport))
    , targetSelector(std::make_unique<TargetSelector>(targetProvider))
    , out({})
{
}

auto ThreadMissionProcessor::init() -> void
{
    targetSelector->init(droneConfig);
    solver->init();

    dropParams = solver->calcDropParameters(droneConfig.ammo, droneConfig.attackSpeed, droneConfig.altitude);

    simulationStep = {.pos = droneConfig.startPos,          // позиція дрона
                      .direction = droneConfig.initialDir,  // напрямок (рад)
                      .state = STOPPED,                     // стан автомата(0 - 4)
                      .targetIdx = -1,                      // індекс поточної цілі
                      .dropPoint = {0.F, 0.F},              // точка скиду (куди летить дрон)
                      .aimPoint = {0.F, 0.F},               // куди впаде бомба (якщо скинути зараз)
                      .predictedTarget = {0.F, 0.F},        // прогнозована позиція цілі
                      .speed = 0.F,
                      .timeSecSinceStart = 0.F};

    context = std::make_unique<DroneContext>(DroneContext{.currentTime = currentTime,
                                                          .simulationStep = &simulationStep,
                                                          .droneConfig = &droneConfig,
                                                          .dronePhysics = dronePhysics,
                                                          .droneTelemetry = {},
                                                          .dropParams = &dropParams,
                                                          .turnAngle = 0.F,
                                                          .acceleration = droneConfig.acceleration(),
                                                          .angleStep = droneConfig.angularSpeed,  // droneConfig.angleStep(),
                                                          .distanceToDropPoint = 0.F});

    states[DroneStatus::STOPPED] = [](TargetSelector& targetSelector) { return std::make_unique<StoppedState>(targetSelector); };
    states[DroneStatus::TURNING] = [](TargetSelector& targetSelector) { return std::make_unique<TurningState>(targetSelector); };
    states[DroneStatus::ACCELERATING] = [](TargetSelector& targetSelector) { return std::make_unique<AcceleratingState>(targetSelector); };
    states[DroneStatus::DECELERATING] = [](TargetSelector& targetSelector) { return std::make_unique<DeceleratingState>(targetSelector); };
    states[DroneStatus::MOVING] = [](TargetSelector& targetSelector) { return std::make_unique<MovingState>(targetSelector); };
};

auto ThreadMissionProcessor::intervalTask() -> void
{
    if (currentStep >= MAX_STEPS) {
        stopRequested = true;

        return;
    }

    context->droneTelemetry = dronePhysics->getTelemetry();

    auto simulationStep = calculateSimulationStep();

    DEBUG(currentStep << context->distanceToDropPoint << " ");
    DEBUG(" Tel:" << context->droneTelemetry.direction << " " << context->droneTelemetry.state << " ");
    DEBUG(context->droneTelemetry.timeSinceStart);

    if (currentStep > 0) {
        std::lock_guard<std::mutex> lock(dataMutex);
        *context->simulationStep = simulationStep;
        out.at(currentStep - 1) = *context->simulationStep;
    }

    if (isTargetHit(simulationStep)) {
        stopRequested = true;

        return;
    }

    auto command = states[context->droneTelemetry.state](*targetSelector)->threadExecute(*context);
    dronePhysics->executeCommand(command);

    context->currentTime += 0.1F;
    currentStep++;
}

ThreadMissionProcessor::~ThreadMissionProcessor() = default;

auto ThreadMissionProcessor::calculateSimulationStep() -> SimStep
{
    auto speed = context->droneTelemetry.speed.toSpeed();
    auto selectedTarget = targetSelector->selectTarget(context->droneTelemetry, *context->dropParams);
    Coord targetPosition = selectedTarget.target->position;
    Speed targetSpeed = selectedTarget.target->velocity;
    auto targetDistance = context->droneTelemetry.position.distance(targetPosition);
    Coord predictedTarget = {
        selectedTarget.position.x + targetSpeed.x * selectedTarget.timeToReachPosition,
        selectedTarget.position.y + targetSpeed.y * selectedTarget.timeToReachPosition,
    };

    DEBUG("time" << selectedTarget.timeToReachPosition);

    context->distanceToDropPoint = context->simulationStep->pos.distance(context->simulationStep->predictedTarget) - dropParams.distance;

    return {.pos = context->droneTelemetry.position,         // позиція дрона
            .direction = context->droneTelemetry.direction,  // напрямок (рад)
            .state = context->droneTelemetry.state,          // стан автомата(0 - 4)
            .targetIdx = selectedTarget.idx,                 // індекс поточної цілі
            .dropPoint = context->simulationStep->pos.move(targetDistance - dropParams.distance,
                                                           context->simulationStep->direction),  // точка скиду (куди летить дрон)
            .aimPoint = context->simulationStep->pos.move(dropParams.distance,
                                                          context->simulationStep->direction),  // куди впаде бомба (якщо скинути зараз)
            .predictedTarget = predictedTarget,  // прогнозована позиція цілі
            .speed = speed,
            .timeSecSinceStart = context->droneTelemetry.timeSinceStart};
}

auto ThreadMissionProcessor::isTargetHit(SimStep& simStep) const -> bool
{
    // check hitRadius
    if (std::abs(simStep.speed - droneConfig.attackSpeed) < epsilon) {
        auto DF = simStep.aimPoint.distance(simStep.predictedTarget);
        DEBUG(" DF:" << DF);

        if (DF <= droneConfig.hitRadius) {
            return true;
        }
    }

    return false;
}

auto ThreadMissionProcessor::dumpResults() -> void
{
    simulationExport->dumpResults(currentStep, out);
}