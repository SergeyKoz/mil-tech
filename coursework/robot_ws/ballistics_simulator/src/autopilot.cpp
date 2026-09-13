#include "ballistics_simulator/autopilot.hpp"
#include "interfaces/ballistics_solver_interface.hpp"
#include "interfaces/targets_provider_interface.hpp"
#include "ballistics_simulator/target_selector.hpp"

#include "ballistics_simulator/states/accelerating_state.hpp"
#include "ballistics_simulator/states/decelerating_state.hpp"
#include "ballistics_simulator/states/moving_state.hpp"
#include "ballistics_simulator/states/stopped_state.hpp"
#include "ballistics_simulator/states/turning_state.hpp"
// #include "DroneAutopilot.hpp"
// #include "interfaces/IConfigLoader.hpp"
// #include "interfaces/ITargetsProvider.hpp"
// #include "config/CheckerConfigLoader.hpp"
// #include "providers/CheckerTargetProvider.hpp"
// #include "states/AcceleratingState.hpp"
// #include "states/DeceleratingState.hpp"
// #include "states/MovingState.hpp"
// #include "states/StoppedState.hpp"
// #include "states/TurningState.hpp"
// #include "rpiChecker/RpiCheckerGPIO.hpp"
// #include "rpiChecker/RpiCheckerUART.hpp"
// #include "TargetSelector.hpp"
// #include <iostream>
// #include "drone_link.h"
// #include <gpiod.h>
// #include <fcntl.h>
// #include <termios.h>
// #include <unistd.h>

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

        log() << "set ammo";

        droneConfig.hitRadius = config.hitRadius;
        droneConfig.ammo = {.name = config.name, .mass = config.mass, .drag = config.drag, .lift = config.lift};

        // dynamic_cast<CheckerTargetProvider *>(targetProvider.get())->setTargetsCount(ammoConfig.nTargets);

        isConfigured = isDroneConfigReady(droneConfig);
    }

    auto Autopilot::isDroneConfigReady(const DroneConfig &droneConfig) -> bool
    {
        return std::abs(droneConfig.altitude) > epsilon && !droneConfig.ammo.name.empty() && std::abs(droneConfig.attackSpeed) > epsilon;
    }

    auto Autopilot::processTelemetry(DroneTelemetry &droneTelemetry) -> void
    {
        // log() << "processTelemetry " << 10 << " start";

        currentTime = droneTelemetry.timeSinceStart;

        if (!isConfigured)
        {
            droneConfig.startPos = droneTelemetry.position;
            droneConfig.altitude = droneTelemetry.altitude;
            droneConfig.initialDir = droneTelemetry.direction;

            //     dynamic_cast<CheckerConfigLoader *>(configLoader.get())->setConfig(droneConfig);
            //     LOG("Drone is ready to start the mission!");

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

            //     isTargetsDefined = dynamic_cast<CheckerTargetProvider *>(targetProvider.get())->isReady();

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

            //     DEBUG("Drop parameters calculated: time=" << dropParams.time << ", distance=" << dropParams.distance);

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

            //     context = std::make_unique<DroneContext>(
            //         DroneContext{.currentTime = 0.F,
            //                      .simulationStep = {},
            //                      .droneConfig = &droneConfig,
            //                      .dronePhysics = {},
            //                      .droneTelemetry = {.state = STOPPED,
            //                                         .position = {telemetry.x, telemetry.y},
            //                                         .altitude = droneConfig.altitude,
            //                                         .speed = {telemetry.vx, telemetry.vy},
            //                                         .direction = telemetry.dir,
            //                                         .timeSinceStart = static_cast<float>(telemetry.t_ms) / 1000.0F},
            //                      .dropParams = &dropParams,
            //                      .selectedTarget = {},
            //                      .turnAngle = 0.F,
            //                      .acceleration = droneConfig.acceleration(),
            //                      .angleStep = droneConfig.angularSpeed,
            //                      .distanceToDropPoint = 0.F});
        }

        if (isConfigured && isTargetsDefined && isDropParametersCalculated)
        {
            droneTelemetry.state = context->droneTelemetry.state;
            context->droneTelemetry = droneTelemetry;

            // context->droneTelemetry = DroneTelemetry{.state = context->droneTelemetry.state,
            //                                          .position = {telemetry.x, telemetry.y},
            //                                          .altitude = droneConfig.altitude,
            //                                          .speed = {telemetry.vx, telemetry.vy},
            //                                          .direction = telemetry.dir,
            //                                          .timeSinceStart = static_cast<float>(telemetry.t_ms) / 1000.0F};

            auto simulationStep = calculateSimulationStep();

            context->simulationStep = simulationStep.get();

            if (isTargetHit(*context))
            {
                throw TargetHit(std::to_string(context->simulationStep->targetIdx));
            }

            auto command = states[context->droneTelemetry.state]()->execute(*context);
            context->droneTelemetry.state = command.state;
            log() << "Command: " << command.state << " acc: " << command.acceleration << " ang: " << command.angleSpeed
                  << " max: " << command.maxSpeed;

            float turnPosition = command.angleSpeed > epsilon ? 1.0F : -1.0F;
            float turnRate = std::abs(command.angleSpeed) < epsilon ? 0.0F : turnPosition;
            float accelPosition = command.state == ACCELERATING ? 1.0F : -1.0F;
            float accel = command.state != ACCELERATING && command.state != DECELERATING ? 0.F : accelPosition;

            ControlCommand controlCommand = {.acceleration = accel, .turnRate = turnRate};
            controlCommandHandler(controlCommand);

            //     rpiCheckerUART->writeControl({.accel = accel, .turnRate = turnRate});
        }
    }

    void Autopilot::setCommandHandler(ControlCommandHandler handler)
    {
        controlCommandHandler = handler ? std::move(handler) : [](const ControlCommand &) {};
    }

    // DroneAutopilot::DroneAutopilot(std::unique_ptr<IBallisticsSolver> solver,
    //                                std::unique_ptr<IConfigLoader> configLoader,
    //                                std::unique_ptr<ITargetsProvider> targetProvider,
    //                                std::unique_ptr<RpiCheckerGPIO> rpiCheckerGPIO,
    //                                std::unique_ptr<RpiCheckerUART> rpiCheckerUART)
    //     : solver(std::move(solver))
    //     , configLoader(std::move(configLoader))
    //     , targetProvider(std::move(targetProvider))
    //     , targetSelector(std::make_unique<TargetSelector>(*this->targetProvider))
    //     , rpiCheckerGPIO(std::move(rpiCheckerGPIO))
    //     , rpiCheckerUART(std::move(rpiCheckerUART))
    //     , currentTime(0.0F)
    //     , uart(0){};

    // auto DroneAutopilot::updateTelemetry(const dlink::Telemetry &telemetry) -> void
    // {
    //     DEBUG("TELEMETRY: " << " state=" << static_cast<int>(telemetry.state) << " x,y=" << telemetry.x << "," << telemetry.y
    //                         << " vx,vy=" << telemetry.vx << "," << telemetry.vy << " dir=" << telemetry.dir << " speed=" << telemetry.speed);

    //     currentTime = static_cast<float>(telemetry.t_ms) / 1000.0F;  // мілісекунди -> секунди

    //     auto droneConfig = configLoader->getConfig();

    //     if (!isConfigured) {
    //         droneConfig.startPos = {telemetry.x, telemetry.y};
    //         droneConfig.altitude = telemetry.z;
    //         droneConfig.initialDir = telemetry.dir;

    //         dynamic_cast<CheckerConfigLoader *>(configLoader.get())->setConfig(droneConfig);
    //         LOG("Drone is ready to start the mission!");

    //         isConfigured = isDroneConfigReady(droneConfig);

    //         if (!isConfigured) {
    //             return;
    //         }
    //     }

    //     if (!isTargetsDefined) {
    //         isTargetsDefined = dynamic_cast<CheckerTargetProvider *>(targetProvider.get())->isReady();

    //         if (!isTargetsDefined) {
    //             return;
    //         }
    //     }

    //     if (isConfigured && isTargetsDefined && !isDropParametersCalculated) {
    //         dropParams = solver->calcDropParameters(droneConfig.ammo, droneConfig.attackSpeed, droneConfig.altitude);
    //         isDropParametersCalculated = true;

    //         DEBUG("Drop parameters calculated: time=" << dropParams.time << ", distance=" << dropParams.distance);

    //         targetSelector->init(droneConfig);

    //         context =
    //             std::make_unique<DroneContext>(DroneContext{.currentTime = 0.F,
    //                                                         .simulationStep = {},
    //                                                         .droneConfig = &droneConfig,
    //                                                         .dronePhysics = {},
    //                                                         .droneTelemetry = {.state = STOPPED,
    //                                                                            .position = {telemetry.x, telemetry.y},
    //                                                                            .altitude = droneConfig.altitude,
    //                                                                            .speed = {telemetry.vx, telemetry.vy},
    //                                                                            .direction = telemetry.dir,
    //                                                                            .timeSinceStart = static_cast<float>(telemetry.t_ms) / 1000.0F},
    //                                                         .dropParams = &dropParams,
    //                                                         .selectedTarget = {},
    //                                                         .turnAngle = 0.F,
    //                                                         .acceleration = droneConfig.acceleration(),
    //                                                         .angleStep = droneConfig.angularSpeed,
    //                                                         .distanceToDropPoint = 0.F});
    //     }

    //     if (isConfigured && isTargetsDefined && isDropParametersCalculated) {
    //         context->droneTelemetry = DroneTelemetry{.state = context->droneTelemetry.state,
    //                                                  .position = {telemetry.x, telemetry.y},
    //                                                  .altitude = droneConfig.altitude,
    //                                                  .speed = {telemetry.vx, telemetry.vy},
    //                                                  .direction = telemetry.dir,
    //                                                  .timeSinceStart = static_cast<float>(telemetry.t_ms) / 1000.0F};

    //         auto simulationStep = calculateSimulationStep();

    //         context->simulationStep = simulationStep.get();

    //         if (isTargetHit(*context)) {
    //             throw TargetHit(std::to_string(context->simulationStep->targetIdx));
    //         }

    //         auto command = states[context->droneTelemetry.state](*targetSelector)->threadExecute(*context);
    //         context->droneTelemetry.state = command.state;
    //         DEBUG("Command: " << command.state << " acc: " << command.acceleration << " ang: " << command.angleSpeed
    //                           << " max: " << command.maxSpeed);

    //         float turnPosition = command.angleSpeed > epsilon ? 1.0F : -1.0F;
    //         float turnRate = std::abs(command.angleSpeed) < epsilon ? 0.0F : turnPosition;

    //         float accelPosition = command.state == ACCELERATING ? 1.0F : -1.0F;
    //         float accel = command.state != ACCELERATING && command.state != DECELERATING ? 0.F : accelPosition;

    //         rpiCheckerUART->writeControl({.accel = accel, .turnRate = turnRate});
    //     }
    // }

    // auto DroneAutopilot::updateTargetPosition(const dlink::TargetPos &targetPosition) -> void
    // {
    //     auto *targets = dynamic_cast<CheckerTargetProvider *>(targetProvider.get());

    //     targets->setTarget(targetPosition.id, {targetPosition.x, targetPosition.y}, currentTime);
    // }

    // auto DroneAutopilot::updateAmmoConfig(const dlink::AmmoCfg &ammoConfig) -> void
    // {
    //     if (isConfigured) {
    //         return;
    //     }

    //     auto droneConfig = configLoader->getConfig();

    //     droneConfig.hitRadius = ammoConfig.hitRadius;
    //     droneConfig.ammo = {.name = &ammoConfig.name[0], .mass = ammoConfig.mass, .drag = ammoConfig.drag, .lift = ammoConfig.lift};

    //     dynamic_cast<CheckerConfigLoader *>(configLoader.get())->setConfig(droneConfig);
    //     dynamic_cast<CheckerTargetProvider *>(targetProvider.get())->setTargetsCount(ammoConfig.nTargets);

    //     isConfigured = isDroneConfigReady(droneConfig);
    // }

    // auto DroneAutopilot::updateResult(const dlink::Result &result) -> void
    // {
    //     DEBUG("Result: (hit: " << result.hit << " miss: " << result.miss_m << " targetId: " << result.targetId << ")");
    // }

    // auto DroneAutopilot::updateDroneConfig(const dlink::DroneCfg &droneConfig) -> void
    // {
    //     if (isConfigured) {
    //         return;
    //     }

    //     auto config = configLoader->getConfig();

    //     config.attackSpeed = droneConfig.attackSpeed;
    //     config.accelerationPath = droneConfig.accelerationPath;
    //     config.angularSpeed = droneConfig.angularSpeed;
    //     config.turnThreshold = droneConfig.turnThreshold;
    //     config.simTimeStep = droneConfig.timeStep;
    //     config.timeScale = droneConfig.timeScale;

    //     dynamic_cast<CheckerConfigLoader *>(configLoader.get())->setConfig(config);

    //     isConfigured = isDroneConfigReady(config);
    // }

    // auto DroneAutopilot::updateControl(const dlink::Control &control) -> void
    // {
    //     DEBUG("Control command: (accel:" << control.accel << " turnRate:" << control.turnRate << ")");
    // }

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
        // DEBUG(" DF:" << DF << " Target: " << droneContext.simulationStep->targetIdx);

        return DF <= droneContext.droneConfig->hitRadius;
    }

    // auto DroneAutopilot::isDroneConfigReady(const DroneConfig &droneConfig) -> bool
    // {
    //     return std::abs(droneConfig.altitude) > epsilon && !droneConfig.ammo.name.empty() && std::abs(droneConfig.attackSpeed) > epsilon;
    // }

    // auto DroneAutopilot::init() -> void
    // {
    //     solver->init();
    //     rpiCheckerGPIO->init();
    //     rpiCheckerUART->init();
    //     rpiCheckerUART->addListener(*this);

    //     states[DroneStatus::STOPPED] = [](TargetSelector &targetSelector) { return std::make_unique<StoppedState>(targetSelector); };
    //     states[DroneStatus::TURNING] = [](TargetSelector &targetSelector) { return std::make_unique<TurningState>(targetSelector); };
    //     states[DroneStatus::ACCELERATING] = [](TargetSelector &targetSelector) { return std::make_unique<AcceleratingState>(targetSelector); };
    //     states[DroneStatus::DECELERATING] = [](TargetSelector &targetSelector) { return std::make_unique<DeceleratingState>(targetSelector); };
    //     states[DroneStatus::MOVING] = [](TargetSelector &targetSelector) { return std::make_unique<MovingState>(targetSelector); };

    //     context = std::make_unique<DroneContext>(DroneContext{});
    // };

    // auto DroneAutopilot::start() -> void
    // {
    //     rpiCheckerGPIO->start();

    //     LOG("Start signal sent!");
    // };

    // auto DroneAutopilot::mission() -> void
    // {
    //     try {
    //         rpiCheckerUART->listenPackages();
    //     }
    //     catch (const TargetHit &e) {
    //         LOG("Target: " << e.what() << " hit!");

    //         return;
    //     }
    // };

    // auto DroneAutopilot::drop() -> void
    // {
    //     rpiCheckerGPIO->drop();

    //     LOG("Drop signal sent!");
    // }

    Autopilot::~Autopilot() = default;
}