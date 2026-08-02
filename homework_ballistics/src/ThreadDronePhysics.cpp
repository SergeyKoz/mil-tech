#include "ThreadDronePhysics.hpp"
#include "common.hpp"

ThreadDronePhysics::ThreadDronePhysics(const DroneConfig& droneConfig)
    : IntervalWorker(std::chrono::milliseconds(static_cast<int>(std::round(droneConfig.simTimeStep * 1000))),
                     static_cast<int>(droneConfig.timeScale))
    , droneTelemetry({
          .state = STOPPED,
          .position = droneConfig.startPos,
          .speed = {0.F, 0.F},
          .direction = droneConfig.initialDir,
          .timeSinceStart = 0.F,
      })
    , droneCommand({.state = STOPPED, .angleSpeed = 0.F, .acceleration = 0.F, .maxSpeed = 0.F})
{
}

auto ThreadDronePhysics::executeCommand(DroneCommand command) -> void
{
    std::lock_guard<std::mutex> lock(dataMutex);

    droneCommand = command;
};

auto ThreadDronePhysics::getTelemetry() -> DroneTelemetry
{
    std::lock_guard<std::mutex> lock(dataMutex);

    return droneTelemetry;
};

auto ThreadDronePhysics::intervalTask() -> void
{
    updateTelemetry();
}

auto ThreadDronePhysics::updateTelemetry() -> void
{
    auto timeStep = static_cast<float>(interval.count()) / 1000.0F;
    auto telemetry = droneTelemetry;
    telemetry.state = droneCommand.state;
    telemetry.timeSinceStart += timeStep;

    switch (droneCommand.state) {
        case DroneStatus::STOPPED:
            break;
        case DroneStatus::TURNING: {
            float direction = std::fmod(telemetry.direction + droneCommand.angleSpeed * timeStep, 2.0F * std::numbers::pi_v<float>);

            if (direction < 0.0F) {
                direction += 2.0F * std::numbers::pi_v<float>;
            }
            telemetry.direction = direction;

            break;
        }
        case DroneStatus::ACCELERATING: {
            float speed = telemetry.speed.toSpeed();
            float path = speed * timeStep + 0.5F * droneCommand.acceleration * timeStep * timeStep;

            speed += droneCommand.acceleration * timeStep;

            if (std::abs(speed - droneCommand.maxSpeed) < epsilon || speed > droneCommand.maxSpeed) {
                speed = droneCommand.maxSpeed;
            }

            telemetry.position = telemetry.position.move(path, telemetry.direction);
            telemetry.speed = telemetry.speed.fromSpeed(speed, telemetry.direction);

            break;
        }
        case DroneStatus::DECELERATING: {
            float speed = telemetry.speed.toSpeed();
            float path = speed * timeStep - 0.5F * droneCommand.acceleration * timeStep * timeStep;

            speed -= droneCommand.acceleration * timeStep;

            if (std::abs(speed) < epsilon) {
                speed = 0.F;
            }

            telemetry.position = telemetry.position.move(path, telemetry.direction);
            telemetry.speed = telemetry.speed.fromSpeed(speed, telemetry.direction);

            break;
        }
        case DroneStatus::MOVING: {
            float path = telemetry.speed.toSpeed() * timeStep;
            telemetry.position = telemetry.position.move(path, telemetry.direction);

            break;
        }
    }

    {
        std::lock_guard<std::mutex> lock(dataMutex);
        droneTelemetry = telemetry;
    }
};

ThreadDronePhysics::~ThreadDronePhysics() = default;