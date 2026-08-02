#pragma once

#include "common.hpp"
#include "IntervalWorker.hpp"

class ThreadDronePhysics : public IntervalWorker {
  public:
    ThreadDronePhysics(const DroneConfig& droneConfig);

    auto executeCommand(DroneCommand command) -> void;
    auto getTelemetry() -> DroneTelemetry;

    ~ThreadDronePhysics();

  private:
    std::mutex dataMutex;

    DroneTelemetry droneTelemetry;
    DroneCommand droneCommand;

    auto intervalTask() -> void override;
    auto updateTelemetry() -> void;
};
