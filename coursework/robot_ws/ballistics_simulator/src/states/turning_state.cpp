#include "ballistics_simulator/states/turning_state.hpp"

namespace ballistics_simulator
{
    auto TurningState::execute(DroneContext &context) -> DroneCommand
    {
        auto telemetry = context.droneTelemetry;
        auto *config = context.droneConfig;
        auto predictedTargetPosition = context.simulationStep->predictedTarget;

        // decision
        float reEntryPath = 0.F;

        if (context.distanceToDropPoint < 0)
        {
            reEntryPath += 2 * config->accelerationPath + std::fabs(context.distanceToDropPoint);
        }

        float angleStep = context.angleStep > config->turnThreshold ? config->turnThreshold : context.angleStep;

        if (reEntryPath > 0)
        {
            float reversDirection = predictedTargetPosition.direction(telemetry.position);
            context.turnAngle = reversDirection - telemetry.direction;
            bool isReverseDirection = std::fabs(context.turnAngle) < angleStep;

            if (isReverseDirection)
            {
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

        if (isNeedTurnAngle)
        {
            return {
                .state = TURNING,
                .angleSpeed = context.turnAngle > 0 ? config->angularSpeed : -config->angularSpeed,
                .acceleration = 0.F,
                .maxSpeed = 0.F,
            };
        }

        return {.state = ACCELERATING, .angleSpeed = 0.F, .acceleration = context.acceleration, .maxSpeed = context.droneConfig->attackSpeed};
    };
} // namespace ballistics_simulator