#include "states/StoppedState.hpp"
#include <memory>
#include "common.hpp"
#include "states/AcceleratingState.hpp"
#include "states/TurningState.hpp"
#include "TargetSelector.hpp"

StoppedState::StoppedState(TargetSelector& targetSelector)
    : targetSelector(&targetSelector)
{
}

auto StoppedState::execute(DroneContext& context) -> std::unique_ptr<IDroneState>
{
    // action
    context.simulationStep->state = STOPPED;

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