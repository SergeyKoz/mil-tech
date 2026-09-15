#pragma once

// #include "MissionProcessor.hpp"
#include "ballistics_simulator/common.hpp"
#include <mutex>
#include <memory>

class ITargetsProvider;

namespace ballistics_simulator
{
  // class Target;
  // struct DroneConfig;
  // class ITargetsProvider;

  struct TargetSnapshot
  {
    int index;
    TargetTelemetry telemetry;
  };

  class TargetSelector
  {
  public:
    TargetSelector(std::shared_ptr<ITargetsProvider> targetsProvider);
    void init(const DroneConfig &droneConfig);
    SelectedTarget selectTarget(DroneTelemetry droneTelemetry, DropParameters dropParameters);

  private:
    std::mutex dataMutex;

    const DroneConfig *droneConfig;
    std::shared_ptr<ITargetsProvider> targetsProvider;
    float acceleration;
    float fullAccelerationTime;
    static float calcReEntryPath(
        float distanceToDropPoint, bool isNeedTurnAngle, float speed, float acceleration, const DroneConfig &droneConfig);
    static float calcEntryTime(float speed,
                               DroneStatus state,
                               float distanceToDropPoint,
                               bool isNeedTurnAngle,
                               float acceleration,
                               float fullAccelerationTime,
                               const DroneConfig &droneConfig);
    static float calcReEntryTime(
        float speed, float reEntryPath, float turnAngle, float acceleration, float fullAccelerationTime, const DroneConfig &droneConfig);
  };

} // namespace ballistics_simulator