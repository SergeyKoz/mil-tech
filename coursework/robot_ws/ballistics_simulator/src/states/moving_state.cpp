#include "ballistics_simulator/states/moving_state.hpp"

namespace ballistics_simulator
{
    auto MovingState::execute(DroneContext &context) -> DroneCommand
    {
        auto telemetry = context.droneTelemetry;
        auto *config = context.droneConfig;
        auto predictedTargetPosition = context.simulationStep->predictedTarget;

        // decision
        float reEntryPath = 0.F;

        if (context.distanceToDropPoint < 0)
        {
            float overflightAftetStop = context.distanceToDropPoint + config->accelerationPath;
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
                return {.state = MOVING, .angleSpeed = 0.F, .acceleration = 0.F, .maxSpeed = context.droneConfig->attackSpeed};
            }

            return {.state = DECELERATING, .angleSpeed = 0.F, .acceleration = context.acceleration, .maxSpeed = 0.F};
        }

        float directionToPredictedTarget = telemetry.position.direction(predictedTargetPosition);
        context.turnAngle = directionToPredictedTarget - telemetry.direction;
        bool isNeedTurnAngle = std::fabs(context.turnAngle) > config->turnThreshold;

        if (isNeedTurnAngle)
        {
            return {.state = DECELERATING, .angleSpeed = 0.F, .acceleration = context.acceleration, .maxSpeed = 0.F};
        }

        return {.state = MOVING, .angleSpeed = 0.F, .acceleration = 0.F, .maxSpeed = context.droneConfig->attackSpeed};
    };
} // namespace ballistics_simulator