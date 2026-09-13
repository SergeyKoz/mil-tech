#include "ballistics_simulator/states/accelerating_state.hpp"
// #include "states/AcceleratingState.hpp"
// #include "TargetSelector.hpp"
// #include "common.hpp"
// #include "states/DeceleratingState.hpp"
// #include "states/MovingState.hpp"

namespace ballistics_simulator
{
    auto AcceleratingState::execute(DroneContext &context) -> DroneCommand
    {
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
                // fly away
                if (std::abs(speed - config->attackSpeed) < epsilon)
                {
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

        if (isNeedTurnAngle)
        {
            return {.state = DECELERATING, .angleSpeed = 0.F, .acceleration = context.acceleration, .maxSpeed = 0.F};
        }

        if (std::abs(speed - context.droneConfig->attackSpeed) < epsilon)
        {
            return {.state = MOVING, .angleSpeed = 0.F, .acceleration = 0.F, .maxSpeed = config->attackSpeed};
        }

        return {.state = ACCELERATING, .angleSpeed = 0.F, .acceleration = context.acceleration, .maxSpeed = config->attackSpeed};
    };

} // namespace ballistics_simulator