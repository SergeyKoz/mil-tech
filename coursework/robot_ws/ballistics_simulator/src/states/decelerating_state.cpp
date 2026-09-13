#include "ballistics_simulator/states/decelerating_state.hpp"
// #include "states/DeceleratingState.hpp"
// #include "TargetSelector.hpp"
// #include "DronePhysics.hpp"
// #include "common.hpp"
// #include "states/AcceleratingState.hpp"
// #include "states/StoppedState.hpp"

namespace ballistics_simulator
{
    auto DeceleratingState::execute(DroneContext &context) -> DroneCommand
    {
        // calc telemetry
        auto telemetry = context.droneTelemetry;
        auto *config = context.droneConfig;
        auto predictedTargetPosition = context.simulationStep->predictedTarget;
        auto speed = telemetry.speed.toSpeed();

        // decision
        float reEntryPath = 0.F;

        if (context.distanceToDropPoint < 0)
        {
            float stopingPath = (speed * speed) / (2 * context.acceleration);
            float overflightAftetStop = context.distanceToDropPoint + stopingPath;
            reEntryPath += 2 * config->accelerationPath + std::fabs(overflightAftetStop);
        }

        // define next state
        if (reEntryPath > 0)
        {
            float reversDirection = predictedTargetPosition.direction(telemetry.position);
            context.turnAngle = reversDirection - telemetry.direction;
            bool isReverseDirection = std::fabs(context.turnAngle) < config->turnThreshold;

            if (isReverseDirection)
            {
                return {.state = ACCELERATING,
                        .angleSpeed = 0.F,
                        .acceleration = context.acceleration,
                        .maxSpeed = context.droneConfig->attackSpeed};
            }

            if (std::abs(speed) > epsilon)
            {
                return {.state = DECELERATING, .angleSpeed = 0.F, .acceleration = context.acceleration, .maxSpeed = 0.F};
            }

            return {.state = STOPPED, .angleSpeed = 0.F, .acceleration = 0.F, .maxSpeed = 0.F};
        }

        float directionToPredictedTarget = telemetry.position.direction(predictedTargetPosition);
        context.turnAngle = directionToPredictedTarget - telemetry.direction;
        bool isNeedTurnAngle = std::fabs(context.turnAngle) > config->turnThreshold;

        if (isNeedTurnAngle)
        {
            if (std::abs(speed) > epsilon)
            {
                return {.state = DECELERATING, .angleSpeed = 0.F, .acceleration = context.acceleration, .maxSpeed = 0.F};
            }

            return {.state = STOPPED, .angleSpeed = 0.F, .acceleration = 0.F, .maxSpeed = 0.F};
        }

        return {.state = ACCELERATING, .angleSpeed = 0.F, .acceleration = context.acceleration, .maxSpeed = context.droneConfig->attackSpeed};
    };
} // namespace ballistics_simulator