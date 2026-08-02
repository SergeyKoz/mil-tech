#pragma once
#include "common.hpp"

class [[deprecated]] DronePhysics {
  public:
    DronePhysics(float timeStep);
    auto init(DroneTelemetry initDroneTelemetry) -> void;
    auto executeCommand(DroneCommand command) -> void;
    auto getTelemetry() -> DroneTelemetry;

  private:
    DroneTelemetry droneTelemetry;
    float timeStep;
};
