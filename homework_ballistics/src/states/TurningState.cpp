#include "states/TurningState.hpp"
#include "states/AcceleratingState.hpp"
#include "TargetSelector.hpp"

TurningState::TurningState(TargetSelector& targetSelector)
    : targetSelector(&targetSelector)
{
}

auto TurningState::execute(DroneContext& context) -> std::unique_ptr<IDroneState>
{
    context.dronePhysics->executeCommand({
        .state = TURNING,
        .angleSpeed = context.turnAngle > 0 ? context.angleStep : -context.angleStep,
        .acceleration = context.acceleration,
        .maxSpeed = 0.F,
    });

    auto telemetry = context.dronePhysics->getTelemetry();
    context.simulationStep->direction = telemetry.direction;
    context.simulationStep->state = telemetry.state;

    // calc telemetry
    // context.calcTelemetry(targetSelector->selectTarget(context.currentTime, telemetry, *context.dropParams));

    auto predictedTargetPosition = context.simulationStep->predictedTarget;

    // decision
    float reEntryPath = 0.F;

    if (context.distanceToDropPoint < 0) {
        reEntryPath += 2 * context.droneConfig->accelerationPath + std::fabs(context.distanceToDropPoint);
    }

    float angleStep = context.angleStep > context.droneConfig->turnThreshold ? context.droneConfig->turnThreshold : context.angleStep;

    if (reEntryPath > 0) {
        float reversDirection = predictedTargetPosition.direction(telemetry.position);
        context.turnAngle = reversDirection - telemetry.direction;
        bool isReverseDirection = std::fabs(context.turnAngle) < angleStep;

        if (isReverseDirection) {
            return std::make_unique<AcceleratingState>(*targetSelector);
        }

        return std::make_unique<TurningState>(*targetSelector);
    }

    float directionToPredictedTarget = telemetry.position.direction(predictedTargetPosition);
    context.turnAngle = directionToPredictedTarget - telemetry.direction;
    bool isNeedTurnAngle = std::fabs(context.turnAngle) > angleStep;

    if (isNeedTurnAngle) {
        return std::make_unique<TurningState>(*targetSelector);
    }

    return std::make_unique<AcceleratingState>(*targetSelector);
};

auto TurningState::threadExecute(DroneContext& context) -> DroneCommand
{
    auto telemetry = context.droneTelemetry;
    auto* config = context.droneConfig;
    auto predictedTargetPosition = context.simulationStep->predictedTarget;

    // decision
    float reEntryPath = 0.F;

    if (context.distanceToDropPoint < 0) {
        reEntryPath += 2 * config->accelerationPath + std::fabs(context.distanceToDropPoint);
    }

    float angleStep = context.angleStep > config->turnThreshold ? config->turnThreshold : context.angleStep;

    if (reEntryPath > 0) {
        float reversDirection = predictedTargetPosition.direction(telemetry.position);
        context.turnAngle = reversDirection - telemetry.direction;
        bool isReverseDirection = std::fabs(context.turnAngle) < angleStep;

        if (isReverseDirection) {
            return {.state = ACCELERATING, .angleSpeed = 0.F, .acceleration = context.acceleration, .maxSpeed = config->attackSpeed};
        }

        return {
            .state = TURNING,
            .angleSpeed = context.turnAngle > 0 ? config->angularSpeed : -config->angularSpeed,
            .acceleration = 0.F,
            .maxSpeed = 0.F,
        };
    }

    float directionToPredictedTarget = telemetry.position.direction(predictedTargetPosition);
    context.turnAngle = directionToPredictedTarget - telemetry.direction;
    bool isNeedTurnAngle = std::fabs(context.turnAngle) > angleStep;

    if (isNeedTurnAngle) {
        return {
            .state = TURNING,
            .angleSpeed = context.turnAngle > 0 ? config->angularSpeed : -config->angularSpeed,
            .acceleration = 0.F,
            .maxSpeed = 0.F,
        };
    }

    return {.state = ACCELERATING, .angleSpeed = 0.F, .acceleration = context.acceleration, .maxSpeed = context.droneConfig->attackSpeed};
};