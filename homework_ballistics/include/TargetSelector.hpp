#pragma once

#include "MissionProcessor.hpp"
#include "common.hpp"

struct Target;
struct DroneConfig;
class ITargetsProvider;

struct SelectedTarget {
    int idx;
    Target* target;
    Coord position;
    float timeToReachPosition;
};

class TargetSelector {
  public:
    TargetSelector(ITargetsProvider& targetProvider);
    void init(const DroneConfig& droneConfig);
    SelectedTarget selectTarget(float currentTime, const SimStep& simulationStep, float dropDistance);

  private:
    const DroneConfig* droneConfig;
    ITargetsProvider* targetProvider;
    float acceleration;
    float fullAccelerationTime;
    float angleStep;
    float calcReEntryPath(float distanceToDropPoint, bool isNeedTurnAngle, float speed, float acceleration, const DroneConfig& droneConfig);
    float calcEntryTime(float speed,
                        DroneStatus state,
                        float distanceToDropPoint,
                        bool isNeedTurnAngle,
                        float acceleration,
                        float fullAccelerationTime,
                        const DroneConfig& droneConfig);
    float calcReEntryTime(
      float speed, float reEntryPath, float turnAngle, float acceleration, float fullAccelerationTime, const DroneConfig& droneConfig);
};
