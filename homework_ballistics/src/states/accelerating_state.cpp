#include "states/AcceleratingState.hpp"
#include "TargetSelector.hpp"
#include "common.hpp"
#include "states/DeceleratingState.hpp"
#include "states/MovingState.hpp"

AcceleratingState::AcceleratingState(TargetSelector& targetSelector)
    : targetSelector(&targetSelector)
{
}

auto AcceleratingState::execute(DroneContext& context) -> std::unique_ptr<IDroneState>
{
    // action
    context.dronePhysics->executeCommand({.state = ACCELERATING,
                                          .angleSpeed = context.angleStep,
                                          .acceleration = context.acceleration,
                                          .maxSpeed = context.droneConfig->attackSpeed});

    auto telemetry = context.dronePhysics->getTelemetry();
    auto speed = telemetry.speed.toSpeed();
    context.simulationStep->pos = telemetry.position;
    context.simulationStep->speed = speed > context.droneConfig->attackSpeed ? context.droneConfig->attackSpeed : speed;
    context.simulationStep->state = telemetry.state;

    // calc telemetry
    // context.calcTelemetry(targetSelector->selectTarget(context.currentTime, telemetry, *context.dropParams));

    auto predictedTargetPosition = context.simulationStep->predictedTarget;

    // decision
    float reEntryPath = 0.F;

    if (context.distanceToDropPoint < 0) {
        float stopingPath = (speed * speed) / (2 * context.acceleration);
        float overflightAftetStop = context.distanceToDropPoint + stopingPath;
        reEntryPath += 2 * context.droneConfig->accelerationPath + std::fabs(overflightAftetStop);
    }

    // define next state
    if (reEntryPath > 0) {
        float reversDirection = predictedTargetPosition.direction(telemetry.position);
        context.turnAngle = reversDirection - telemetry.direction;
        bool isReverseDirection = std::fabs(context.turnAngle) < context.droneConfig->turnThreshold;

        if (isReverseDirection) {
            // fly away
            if (std::abs(speed - context.droneConfig->attackSpeed) < epsilon) {
                return std::make_unique<MovingState>(*targetSelector);
            }

            return std::make_unique<AcceleratingState>(*targetSelector);
        }

        // need turn to achieve reverse direction
        return std::make_unique<DeceleratingState>(*targetSelector);
    }

    float directionToPredictedTarget = telemetry.position.direction(predictedTargetPosition);
    context.turnAngle = directionToPredictedTarget - telemetry.direction;
    bool isNeedTurnAngle = std::fabs(context.turnAngle) > context.droneConfig->turnThreshold;

    if (isNeedTurnAngle) {
        return std::make_unique<DeceleratingState>(*targetSelector);
    }

    if (std::abs(speed - context.droneConfig->attackSpeed) < epsilon) {
        return std::make_unique<MovingState>(*targetSelector);
    }

    return std::make_unique<AcceleratingState>(*targetSelector);
};

auto AcceleratingState::threadExecute(DroneContext& context) -> DroneCommand
{
    auto telemetry = context.droneTelemetry;
    auto* config = context.droneConfig;
    auto predictedTargetPosition = context.simulationStep->predictedTarget;
    auto speed = telemetry.speed.toSpeed();

    // decision
    float reEntryPath = 0.F;

    if (context.distanceToDropPoint < 0) {
        float stopingPath = (speed * speed) / (2 * context.acceleration);
        float overflightAftetStop = context.distanceToDropPoint + stopingPath;
        reEntryPath += 2 * config->accelerationPath + std::fabs(overflightAftetStop);
    }

    // define next state
    if (reEntryPath > 0) {
        float reversDirection = predictedTargetPosition.direction(telemetry.position);
        context.turnAngle = reversDirection - telemetry.direction;
        bool isReverseDirection = std::fabs(context.turnAngle) < config->turnThreshold;

        if (isReverseDirection) {
            // fly away
            if (std::abs(speed - config->attackSpeed) < epsilon) {
                return {.state = MOVING, .angleSpeed = 0.F, .acceleration = 0.F, .maxSpeed = config->attackSpeed};
            }

            return {.state = ACCELERATING, .angleSpeed = 0.F, .acceleration = context.acceleration, .maxSpeed = config->attackSpeed};
        }

        // need turn to achieve reverse direction
        return {.state = DECELERATING, .angleSpeed = 0.F, .acceleration = context.acceleration, .maxSpeed = 0.F};
    }

    float directionToPredictedTarget = telemetry.position.direction(predictedTargetPosition);
    context.turnAngle = directionToPredictedTarget - telemetry.direction;
    bool isNeedTurnAngle = std::fabs(context.turnAngle) > config->turnThreshold;

    if (isNeedTurnAngle) {
        return {.state = DECELERATING, .angleSpeed = 0.F, .acceleration = context.acceleration, .maxSpeed = 0.F};
    }

    if (std::abs(speed - context.droneConfig->attackSpeed) < epsilon) {
        return {.state = MOVING, .angleSpeed = 0.F, .acceleration = 0.F, .maxSpeed = config->attackSpeed};
    }

    return {.state = ACCELERATING, .angleSpeed = 0.F, .acceleration = context.acceleration, .maxSpeed = config->attackSpeed};
};