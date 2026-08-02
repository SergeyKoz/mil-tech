#include "MissionProcessor.hpp"
#include <utility>
#include "TargetSelector.hpp"
#include "interfaces/ITargetsProvider.hpp"
#include "interfaces/ISimulationExport.hpp"
#include "interfaces/IBallisticsSolver.hpp"
#include "states/StoppedState.hpp"

MissionProcessor::MissionProcessor(std::unique_ptr<IBallisticsSolver> solver,
                                   const DroneConfig& droneConfig,
                                   std::unique_ptr<ITargetsProvider> targetProvider,
                                   std::unique_ptr<ISimulationExport> simulationExport)
    : out({})
    , droneConfig(droneConfig)
    , dronePhysics(std::make_unique<ThreadDronePhysics>(droneConfig))
    , simulationStep{}
    , solver(std::move(solver))
    , targetProvider(std::move(targetProvider))
    , simulationExport(std::move(simulationExport))
    , currentStep(0)
    , currentTime(0.F)
    , dropParams{}
    , targetSelector(std::make_unique<TargetSelector>(*this->targetProvider)){};

auto MissionProcessor::init() -> void
{
    targetProvider->load();
    targetSelector->init(droneConfig);
    solver->init();

    currentStep = 0;
    currentTime = 0.F;

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
                                                          .dronePhysics = dronePhysics.get(),
                                                          .droneTelemetry = {},
                                                          .dropParams = &dropParams,
                                                          .turnAngle = 0.F,
                                                          .acceleration = droneConfig.acceleration(),
                                                          .angleStep = 0.F,
                                                          .distanceToDropPoint = 0.F});

    state = std::make_unique<StoppedState>(*targetSelector);
};

auto MissionProcessor::hasNext() -> bool
{
    if (currentStep >= MAX_STEPS + 1) {
        return false;
    }

    if (currentStep > 0) {
        out.at(currentStep - 1) = *context->simulationStep;

        if (isTargetHit(*context->simulationStep)) {
            return false;
        }
    }

    return true;
};

auto MissionProcessor::step() -> void
{
    auto next = state->execute(*context);

    if (next) {
        state = std::move(next);
    }

    context->currentTime += 0.1F;
    currentStep++;
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
                      .speed = 0.F,
                      .timeSecSinceStart = 0.F};
};

auto MissionProcessor::changeSolver(std::unique_ptr<IBallisticsSolver> solver) -> void
{
    this->solver = std::move(solver);
}

auto MissionProcessor::isTargetHit(SimStep& simStep) const -> bool
{
    // check hitRadius
    if (std::abs(simStep.speed - droneConfig.attackSpeed) < epsilon) {
        auto DF = simStep.aimPoint.distance(simStep.predictedTarget);

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

MissionProcessor::~MissionProcessor() = default;
