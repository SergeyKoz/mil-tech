#include "states/DeceleratingState.hpp"
#include "TargetSelector.hpp"
#include "states/AcceleratingState.hpp"
#include "states/StoppedState.hpp"

DeceleratingState::DeceleratingState(TargetSelector& targetSelector)
    : targetSelector(&targetSelector)
{
}

auto DeceleratingState::execute(DroneContext& context) -> std::unique_ptr<IDroneState>
{
    // action
    context.simulationStep->state = DECELERATING;
    float path = context.simulationStep->speed * context.droneConfig->simTimeStep -
                 0.5F * context.acceleration * context.droneConfig->simTimeStep * context.droneConfig->simTimeStep;
    context.simulationStep->pos = context.simulationStep->pos.move(path, context.simulationStep->direction);
    context.simulationStep->speed -= context.acceleration * context.droneConfig->simTimeStep;

    if (context.simulationStep->speed <= 0.F) {
        context.simulationStep->speed = 0.F;
    }

    // calc telemetry
    context.calcTelemetry(targetSelector->selectTarget(context.currentTime, *context.simulationStep, *context.dropParams));

    // decision
    float reEntryPath = 0.F;

    if (context.distanceToDropPoint < 0) {
        float stopingPath = (context.simulationStep->speed * context.simulationStep->speed) / (2 * context.acceleration);
        float overflightAftetStop = context.distanceToDropPoint + stopingPath;
        reEntryPath += 2 * context.droneConfig->accelerationPath + std::fabs(overflightAftetStop);
    }

    // define next state
    if (reEntryPath > 0) {
        float reversDirection = context.simulationStep->predictedTarget.direction(context.simulationStep->pos);
        context.turnAngle = reversDirection - context.simulationStep->direction;
        bool isReverseDirection = std::fabs(context.turnAngle) < context.droneConfig->turnThreshold;

        if (isReverseDirection) {
            return std::make_unique<AcceleratingState>(*targetSelector);
        }

        if (context.simulationStep->speed > 0.F) {
            return std::make_unique<DeceleratingState>(*targetSelector);
        }

        return std::make_unique<StoppedState>(*targetSelector);
    }

    float directionToPredictedTarget = context.simulationStep->pos.direction(context.simulationStep->predictedTarget);
    context.turnAngle = directionToPredictedTarget - context.simulationStep->direction;
    bool isNeedTurnAngle = std::fabs(context.turnAngle) > context.droneConfig->turnThreshold;

    if (isNeedTurnAngle) {
        if (context.simulationStep->speed > 0.F) {
            return std::make_unique<DeceleratingState>(*targetSelector);
        }

        return std::make_unique<StoppedState>(*targetSelector);
    }

    return std::make_unique<AcceleratingState>(*targetSelector);
};