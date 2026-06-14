#include "states/TurningState.hpp"
#include "states/AcceleratingState.hpp"
#include "TargetSelector.hpp"

TurningState::TurningState(TargetSelector& targetSelector)
    : targetSelector(&targetSelector)
{
}

auto TurningState::execute(DroneContext& context) -> std::unique_ptr<IDroneState>
{
    // step action
    context.simulationStep->state = TURNING;
    context.simulationStep->turn(context.turnAngle > 0 ? context.angleStep : -context.angleStep);

    // calc telemetry
    context.calcTelemetry(targetSelector->selectTarget(context.currentTime, *context.simulationStep, *context.dropParams));

    // decision
    float reEntryPath = 0.F;

    if (context.distanceToDropPoint < 0) {
        reEntryPath += 2 * context.droneConfig->accelerationPath + std::fabs(context.distanceToDropPoint);
    }

    float angleStep = context.angleStep > context.droneConfig->turnThreshold ? context.droneConfig->turnThreshold : context.angleStep;

    if (reEntryPath > 0) {
        float reversDirection = context.simulationStep->predictedTarget.direction(context.simulationStep->pos);
        context.turnAngle = reversDirection - context.simulationStep->direction;
        bool isReverseDirection = std::fabs(context.turnAngle) < angleStep;

        if (isReverseDirection) {
            return std::make_unique<AcceleratingState>(*targetSelector);
        }

        return std::make_unique<TurningState>(*targetSelector);
    }

    float directionToPredictedTarget = context.simulationStep->pos.direction(context.simulationStep->predictedTarget);
    context.turnAngle = directionToPredictedTarget - context.simulationStep->direction;
    bool isNeedTurnAngle = std::fabs(context.turnAngle) > angleStep;

    if (isNeedTurnAngle) {
        return std::make_unique<TurningState>(*targetSelector);
    }

    return std::make_unique<AcceleratingState>(*targetSelector);
};
