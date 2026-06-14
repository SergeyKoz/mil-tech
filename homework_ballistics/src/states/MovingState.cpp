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
    // action
    context.simulationStep->state = MOVING;
    float path = context.simulationStep->speed * context.droneConfig->simTimeStep;
    context.simulationStep->pos = context.simulationStep->pos.move(path, context.simulationStep->direction);

    // calc telemetry
    context.calcTelemetry(targetSelector->selectTarget(context.currentTime, *context.simulationStep, *context.dropParams));

    // decision
    float reEntryPath = 0.F;

    if (context.distanceToDropPoint < 0) {
        float overflightAftetStop = context.distanceToDropPoint + context.droneConfig->accelerationPath;
        reEntryPath += 2 * context.droneConfig->accelerationPath + std::fabs(overflightAftetStop);
    }

    // define next state
    if (reEntryPath > 0) {
        float reversDirection = context.simulationStep->predictedTarget.direction(context.simulationStep->pos);
        context.turnAngle = reversDirection - context.simulationStep->direction;
        bool isReverseDirection = std::fabs(context.turnAngle) < context.droneConfig->turnThreshold;

        if (isReverseDirection) {
            return std::make_unique<MovingState>(*targetSelector);
        }

        return std::make_unique<DeceleratingState>(*targetSelector);
    }

    float directionToPredictedTarget = context.simulationStep->pos.direction(context.simulationStep->predictedTarget);
    context.turnAngle = directionToPredictedTarget - context.simulationStep->direction;
    bool isNeedTurnAngle = std::fabs(context.turnAngle) > context.droneConfig->turnThreshold;

    if (isNeedTurnAngle) {
        return std::make_unique<DeceleratingState>(*targetSelector);
    }

    return std::make_unique<MovingState>(*targetSelector);
};