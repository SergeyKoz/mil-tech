#include "DronePhysics.hpp"
#include "common.hpp"

DronePhysics::DronePhysics(float timeStep)
    : droneTelemetry({})
    , timeStep(timeStep)
{
}

auto DronePhysics::init(DroneTelemetry initDroneTelemetry) -> void
{
    droneTelemetry = initDroneTelemetry;
};

auto DronePhysics::executeCommand(DroneCommand command) -> void
{
    droneTelemetry.state = command.state;
    droneTelemetry.timeSinceStart += timeStep;

    switch (command.state) {
        case DroneStatus::STOPPED:
            break;
        case DroneStatus::TURNING: {
            float direction = std::fmod(droneTelemetry.direction + command.angleSpeed, 2.0F * std::numbers::pi_v<float>);

            if (direction < 0.0F) {
                direction += 2.0F * std::numbers::pi_v<float>;
            }
            droneTelemetry.direction = direction;

            break;
        }
        case DroneStatus::ACCELERATING: {
            float speed = droneTelemetry.speed.toSpeed();
            float path = speed * timeStep + 0.5F * command.acceleration * timeStep * timeStep;
            droneTelemetry.position = droneTelemetry.position.move(path, droneTelemetry.direction);
            droneTelemetry.speed = droneTelemetry.speed.fromSpeed(speed + command.acceleration * timeStep, droneTelemetry.direction);

            break;
        }
        case DroneStatus::DECELERATING: {
            float speed = droneTelemetry.speed.toSpeed();
            float path = speed * timeStep - 0.5F * command.acceleration * timeStep * timeStep;
            droneTelemetry.position = droneTelemetry.position.move(path, droneTelemetry.direction);
            droneTelemetry.speed = droneTelemetry.speed.fromSpeed(speed - command.acceleration * timeStep, droneTelemetry.direction);

            break;
        }
        case DroneStatus::MOVING: {
            float path = droneTelemetry.speed.toSpeed() * timeStep;
            droneTelemetry.position = droneTelemetry.position.move(path, droneTelemetry.direction);

            break;
        }
    }
};

auto DronePhysics::getTelemetry() -> DroneTelemetry
{
    return droneTelemetry;
};
