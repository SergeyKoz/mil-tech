#include "states/MovingState.hpp"
#include "TargetSelector.hpp"
#include "common.hpp"
#include "states/DeceleratingState.hpp"

MovingState::MovingState(TargetSelector& targetSelector)
    : targetSelector(&targetSelector)
{
}

auto MovingState::execute(DroneContext& context) -> std::unique_ptr<IDroneState>
{
    context.dronePhysics->executeCommand({.state = MOVING,
                                          .angleSpeed = context.angleStep,
                                          .acceleration = context.acceleration,
                                          .maxSpeed = context.droneConfig->attackSpeed});

    auto telemetry = context.dronePhysics->getTelemetry();
    context.simulationStep->pos = telemetry.position;
    context.simulationStep->state = telemetry.state;

    // calc telemetry
    // context.calcTelemetry(targetSelector->selectTarget(context.currentTime, telemetry, *context.dropParams));
    auto predictedTargetPosition = context.simulationStep->predictedTarget;

    // decision
    float reEntryPath = 0.F;

    if (context.distanceToDropPoint < 0) {
        float overflightAftetStop = context.distanceToDropPoint + context.droneConfig->accelerationPath;
        reEntryPath += 2 * context.droneConfig->accelerationPath + std::fabs(overflightAftetStop);
    }

    // define next state
    if (reEntryPath > 0) {
        float reversDirection = predictedTargetPosition.direction(telemetry.position);
        context.turnAngle = reversDirection - telemetry.direction;
        bool isReverseDirection = std::fabs(context.turnAngle) < context.droneConfig->turnThreshold;

        if (isReverseDirection) {
            return std::make_unique<MovingState>(*targetSelector);
        }

        return std::make_unique<DeceleratingState>(*targetSelector);
    }

    float directionToPredictedTarget = telemetry.position.direction(predictedTargetPosition);
    context.turnAngle = directionToPredictedTarget - telemetry.direction;
    bool isNeedTurnAngle = std::fabs(context.turnAngle) > context.droneConfig->turnThreshold;

    if (isNeedTurnAngle) {
        return std::make_unique<DeceleratingState>(*targetSelector);
    }

    return std::make_unique<MovingState>(*targetSelector);
};

auto MovingState::threadExecute(DroneContext& context) -> DroneCommand
{
    auto telemetry = context.droneTelemetry;
    auto* config = context.droneConfig;
    auto predictedTargetPosition = context.simulationStep->predictedTarget;

    // decision
    float reEntryPath = 0.F;

    if (context.distanceToDropPoint < 0) {
        float overflightAftetStop = context.distanceToDropPoint + config->accelerationPath;
        reEntryPath += 2 * config->accelerationPath + std::fabs(overflightAftetStop);
    }

    // define next state
    if (reEntryPath > 0) {
        float reversDirection = predictedTargetPosition.direction(telemetry.position);
        context.turnAngle = reversDirection - telemetry.direction;
        bool isReverseDirection = std::fabs(context.turnAngle) < config->turnThreshold;

        if (isReverseDirection) {
            return {.state = MOVING, .angleSpeed = 0.F, .acceleration = 0.F, .maxSpeed = context.droneConfig->attackSpeed};
        }

        return {.state = DECELERATING, .angleSpeed = 0.F, .acceleration = context.acceleration, .maxSpeed = 0.F};
    }

    float directionToPredictedTarget = telemetry.position.direction(predictedTargetPosition);
    context.turnAngle = directionToPredictedTarget - telemetry.direction;
    bool isNeedTurnAngle = std::fabs(context.turnAngle) > config->turnThreshold;

    if (isNeedTurnAngle) {
        return {.state = DECELERATING, .angleSpeed = 0.F, .acceleration = context.acceleration, .maxSpeed = 0.F};
    }

    return {.state = MOVING, .angleSpeed = 0.F, .acceleration = 0.F, .maxSpeed = context.droneConfig->attackSpeed};
};