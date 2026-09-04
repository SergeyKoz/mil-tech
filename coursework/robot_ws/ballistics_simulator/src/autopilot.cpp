#include "ballistics_simulator/autopilot.hpp"
#include "interfaces/ballistics_solver_interface.hpp"
#include "interfaces/targets_provider_interface.hpp"
#include "ballistics_simulator/target_selector.hpp"
#include "ballistics_simulator/states/accelerating_state.hpp"
#include "ballistics_simulator/states/decelerating_state.hpp"
#include "ballistics_simulator/states/moving_state.hpp"
#include "ballistics_simulator/states/stopped_state.hpp"
#include "ballistics_simulator/states/turning_state.hpp"

namespace ballistics_simulator
{
    Autopilot::Autopilot(std::unique_ptr<IBallisticsSolver> solver, std::shared_ptr<ITargetsProvider> targetsProvider)
        : solver(std::move(solver)), targetsProvider(targetsProvider), targetSelector(std::make_unique<TargetSelector>(targetsProvider)) {};

    auto Autopilot::getCurrentTime() -> float
    {
        return currentTime;
    };

    auto Autopilot::setConfig(const DroneConfig &config) -> void
    {
        if (isConfigured)
        {
            return;
        }

        droneConfig.attackSpeed = config.attackSpeed;
        droneConfig.accelerationPath = config.accelerationPath;
        droneConfig.angularSpeed = config.angularSpeed;
        droneConfig.turnThreshold = config.turnThreshold;
        droneConfig.simTimeStep = config.simTimeStep;
        droneConfig.timeScale = config.timeScale;

        states[DroneStatus::STOPPED] = []()
        { return std::make_unique<StoppedState>(); };
        states[DroneStatus::TURNING] = []()
        { return std::make_unique<TurningState>(); };
        states[DroneStatus::ACCELERATING] = []()
        { return std::make_unique<AcceleratingState>(); };
        states[DroneStatus::DECELERATING] = []()
        { return std::make_unique<DeceleratingState>(); };
        states[DroneStatus::MOVING] = []()
        { return std::make_unique<MovingState>(); };

        isConfigured = isDroneConfigReady(config);
    };

    auto Autopilot::setAmmo(const AmmoConfig &config) -> void
    {
        if (isConfigured)
        {
            return;
        }

        log() << "Set ammo";

        droneConfig.hitRadius = config.hitRadius;
        droneConfig.ammo = {.name = config.name, .mass = config.mass, .drag = config.drag, .lift = config.lift};
        isConfigured = isDroneConfigReady(droneConfig);
    }

    auto Autopilot::isDroneConfigReady(const DroneConfig &droneConfig) -> bool
    {
        return std::abs(droneConfig.altitude) > epsilon && !droneConfig.ammo.name.empty() && std::abs(droneConfig.attackSpeed) > epsilon;
    }

    auto Autopilot::processTelemetry(DroneTelemetry &droneTelemetry) -> void
    {
        currentTime = droneTelemetry.timeSinceStart;

        if (!isConfigured)
        {
            droneConfig.startPos = droneTelemetry.position;
            droneConfig.altitude = droneTelemetry.altitude;
            droneConfig.initialDir = droneTelemetry.direction;
            isConfigured = isDroneConfigReady(droneConfig);

            log() << "droneConfig.altitude " << droneConfig.altitude << " droneConfig.ammo.name " << droneConfig.ammo.name;
            log() << "isConfigured " << isConfigured;

            if (!isConfigured)
            {
                return;
            }

            solver->init();
        }

        if (!isTargetsDefined)
        {
            isTargetsDefined = targetsProvider->isReady();

            if (!isTargetsDefined)
            {
                return;
            }

            log() << "isTargetsDefined yes";
        }

        if (isConfigured && isTargetsDefined && !isDropParametersCalculated)
        {
            dropParams = solver->calcDropParameters(droneConfig.ammo, droneConfig.attackSpeed, droneConfig.altitude);
            isDropParametersCalculated = true;

            log() << "dropParams d: " << dropParams.distance << " t: " << dropParams.time;

            targetSelector->init(droneConfig);
            droneTelemetry.state = STOPPED;

            context = std::make_unique<DroneContext>(
                DroneContext{.currentTime = 0.F,
                             .simulationStep = {},
                             .droneConfig = &droneConfig,
                             .droneTelemetry = droneTelemetry,
                             .dropParams = &dropParams,
                             .selectedTarget = {},
                             .turnAngle = 0.F,
                             .acceleration = droneConfig.acceleration(),
                             .angleStep = droneConfig.angularSpeed,
                             .distanceToDropPoint = 0.F});
        }

        if (isConfigured && isTargetsDefined && isDropParametersCalculated)
        {
            droneTelemetry.state = context->droneTelemetry.state;
            context->droneTelemetry = droneTelemetry;
            auto simulationStep = calculateSimulationStep();
            context->simulationStep = simulationStep.get();

            if (isTargetHit(*context))
            {
                throw TargetHit(std::to_string(context->simulationStep->targetIdx));
            }

            auto command = states[context->droneTelemetry.state]()->execute(*context);

            log() << "Command: " << command.state << " prev state: " << context->droneTelemetry.state << " acc: " << command.acceleration << " ang: " << command.angleSpeed
                  << " max: " << command.maxSpeed;

            context->droneTelemetry.state = command.state;

            float turnPosition = command.angleSpeed > epsilon ? 1.0F : -1.0F;
            float turnRate = std::abs(command.angleSpeed) < epsilon ? 0.0F : turnPosition;
            float accelPosition = command.state == ACCELERATING ? 1.0F : -1.0F;
            float accel = command.state != ACCELERATING && command.state != DECELERATING ? 0.F : accelPosition;

            ControlCommand controlCommand = {.acceleration = accel, .turnRate = turnRate};
            controlCommandHandler(controlCommand);
        }
    }

    void Autopilot::setCommandHandler(ControlCommandHandler handler)
    {
        controlCommandHandler = handler ? std::move(handler) : [](const ControlCommand &) {};
    }

    auto Autopilot::calculateSimulationStep() -> std::unique_ptr<SimStep>
    {
        auto speed = context->droneTelemetry.speed.toSpeed();
        auto [index, targetTelemetry, timeToReachPosition] = targetSelector->selectTarget(context->droneTelemetry, *context->dropParams);
        Coord targetPosition = targetTelemetry.position;
        Speed targetSpeed = targetTelemetry.speed;
        auto dronePosition = context->droneTelemetry.position;
        auto droneDirection = context->droneTelemetry.direction;
        auto targetDistance = context->droneTelemetry.position.distance(targetPosition);
        Coord predictedTarget = {
            targetPosition.x + targetSpeed.x * timeToReachPosition,
            targetPosition.y + targetSpeed.y * timeToReachPosition,
        };

        log() << "SimStep: droneDirection: " << droneDirection << " target:" << index << " target pos: " << targetPosition.x << "," << targetPosition.x << " time: " << timeToReachPosition;

        context->distanceToDropPoint = dronePosition.distance(predictedTarget) - dropParams.distance;

        return std::make_unique<SimStep>(
            SimStep({.pos = dronePosition,                                                                  // позиція дрона
                     .direction = droneDirection,                                                           // напрямок (рад)
                     .state = context->droneTelemetry.state,                                                // стан автомата(0 - 4)
                     .targetIdx = index,                                                                    // індекс поточної цілі
                     .dropPoint = dronePosition.move(targetDistance - dropParams.distance, droneDirection), // точка скиду (куди летить дрон)
                     .aimPoint = dronePosition.move(dropParams.distance, droneDirection),                   // куди впаде бомба (якщо скинути зараз)
                     .predictedTarget = predictedTarget,                                                    // прогнозована позиція цілі
                     .speed = speed,
                     .timeSecSinceStart = context->droneTelemetry.timeSinceStart}));
    }

    auto Autopilot::isTargetHit(const DroneContext &droneContext) -> bool
    {
        if (std::abs(droneContext.simulationStep->speed - droneContext.droneConfig->attackSpeed) > epsilon)
        {
            return false;
        }

        auto DF = droneContext.simulationStep->aimPoint.distance(droneContext.simulationStep->predictedTarget);

        return DF <= droneContext.droneConfig->hitRadius;
    }

    Autopilot::~Autopilot() = default;
}