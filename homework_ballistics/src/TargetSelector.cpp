
#include "TargetSelector.hpp"
#include "Target.hpp"
#include <cmath>
#include "common.hpp"
#include "interfaces/ITargetsProvider.hpp"

TargetSelector::TargetSelector(ITargetsProvider& targetProvider)
    : droneConfig(nullptr)
    , targetProvider(&targetProvider)
    , acceleration(0.F)
    , fullAccelerationTime(0.F)
{
}

void TargetSelector::init(const DroneConfig& droneConfig)
{
    this->droneConfig = &droneConfig;
    acceleration = droneConfig.acceleration();
    fullAccelerationTime = droneConfig.fullAccelerationTime();
};

auto TargetSelector::selectTarget(const DroneTelemetry& droneTelemetry, DropParameters dropParameters) -> SelectedTarget
{
    float minTotalTime = 0.F;
    int selectedTargetIndex = -1;
    Target* selectedTarget = nullptr;
    Target* target = nullptr;
    Coord currentTargetPos{};
    Coord selectedTargetPos{};

    float droneSpeed = droneTelemetry.speed.toSpeed();

    for (int targetIndex = 0; targetIndex < targetProvider->getTargetsCount(); targetIndex++) {
        target = targetProvider->getTarget(targetIndex);
        currentTargetPos = target->position;

        // define total time to reach target
        // totalTime time to stop + time to turn + time to accelerate + time to move
        float timeToTurn{0.F};

        // time to stop + time to turn + time to accelerate + time to move
        bool isNeedTurnAngle = false;
        float targetDirection = droneTelemetry.position.direction(currentTargetPos);
        float turnAngle = targetDirection - droneTelemetry.direction;

        if (std::fabs(turnAngle) > droneConfig->turnThreshold) {
            timeToTurn = std::abs(turnAngle) / droneConfig->angularSpeed;
            isNeedTurnAngle = true;
        }

        // drop point calculation
        float distanceToTarget = currentTargetPos.distance(droneTelemetry.position);
        float distanceToDropPoint = distanceToTarget - dropParameters.distance;

        float reEntryPath = calcReEntryPath(distanceToDropPoint, isNeedTurnAngle, droneSpeed, acceleration, *droneConfig);

        float totalTime = reEntryPath > 0
                              ? calcReEntryTime(droneSpeed, reEntryPath, turnAngle, acceleration, fullAccelerationTime, *droneConfig)
                              : calcEntryTime(droneSpeed,
                                              droneTelemetry.state,
                                              distanceToDropPoint,
                                              isNeedTurnAngle,
                                              acceleration,
                                              fullAccelerationTime,
                                              *droneConfig);

        totalTime += timeToTurn + dropParameters.time;

        if (totalTime < 0) {
            continue;
        }

        if (selectedTargetIndex == -1 || totalTime < minTotalTime) {
            minTotalTime = totalTime;
            selectedTarget = target;
            selectedTargetIndex = targetIndex;
            selectedTargetPos = currentTargetPos;
        }
    }

    return {.idx = selectedTargetIndex, .target = selectedTarget, .position = selectedTargetPos, .timeToReachPosition = minTotalTime};
};

auto TargetSelector::calcReEntryPath(
    float distanceToDropPoint, bool isNeedTurnAngle, float speed, float acceleration, const DroneConfig& droneConfig) -> float
{
    // define reentry path
    float reEntryPath = distanceToDropPoint < 0 ? -distanceToDropPoint : 0.F;

    if (!isNeedTurnAngle) {
        if (speed < droneConfig.attackSpeed) {
            float accelerationPath = (droneConfig.attackSpeed * droneConfig.attackSpeed - speed * speed) / (2 * acceleration);

            if (distanceToDropPoint - accelerationPath < 0) {
                reEntryPath += accelerationPath;
            }
        }
    }
    else {
        float stopingPath = 0.F;

        if (speed > 0) {
            stopingPath = (speed * speed) / (2 * acceleration);
        }

        if (distanceToDropPoint < stopingPath + droneConfig.accelerationPath) {
            reEntryPath = reEntryPath + stopingPath + droneConfig.accelerationPath;
        }
    }

    return reEntryPath;
};

auto TargetSelector::calcEntryTime(float speed,
                                   DroneStatus state,
                                   float distanceToDropPoint,
                                   bool isNeedTurnAngle,
                                   float acceleration,
                                   float fullAccelerationTime,
                                   const DroneConfig& droneConfig) -> float
{
    float timeToStop{0.F};
    float timeToAccelerate{0.F};
    float timeToMove{0.F};

    // target Entry calculation
    if (state == STOPPED || state == TURNING) {
        timeToAccelerate = fullAccelerationTime;
        timeToMove = (distanceToDropPoint - droneConfig.accelerationPath) / droneConfig.attackSpeed;
    }

    if (!isNeedTurnAngle) {
        if (state == ACCELERATING || state == DECELERATING) {
            // calculate path to accelerate from current speed
            timeToAccelerate = (droneConfig.attackSpeed - speed) / acceleration;
            float accelerationPath = (droneConfig.attackSpeed * droneConfig.attackSpeed - speed * speed) / (2 * acceleration);
            timeToMove = (distanceToDropPoint - accelerationPath) / droneConfig.attackSpeed;
        }
    }
    else {
        if (state == MOVING) {
            timeToStop = fullAccelerationTime;
            timeToAccelerate = fullAccelerationTime;
            timeToMove = (distanceToDropPoint - droneConfig.accelerationPath - droneConfig.accelerationPath) / droneConfig.attackSpeed;
        }

        if (state == DECELERATING || state == ACCELERATING) {
            timeToStop = speed / acceleration;
            timeToAccelerate = fullAccelerationTime;
            float stopingPath = (speed * speed) / (2 * acceleration);
            timeToMove = (distanceToDropPoint - stopingPath - droneConfig.accelerationPath) / droneConfig.attackSpeed;
        }
    }

    return timeToStop + timeToAccelerate + timeToMove;
};

auto TargetSelector::calcReEntryTime(float speed,
                                     float reEntryPath,
                                     float turnAngle,
                                     float acceleration,
                                     float fullAccelerationTime,
                                     const DroneConfig& droneConfig) -> float
{
    auto reEntryTimeToTurn = static_cast<float>((2 * M_PI)) / droneConfig.angularSpeed + turnAngle;  // 360
    float timeToStop = 0.F;

    if (speed > 0) {
        timeToStop = speed / acceleration;
    }

    float maneuverTime = NAN;

    if (reEntryPath > 2 * droneConfig.accelerationPath) {
        maneuverTime =
            fullAccelerationTime + fullAccelerationTime + (reEntryPath - 2 * droneConfig.accelerationPath) / droneConfig.attackSpeed;
    }
    else {
        maneuverTime = std::sqrt(reEntryPath / acceleration) + fullAccelerationTime;
    }

    return timeToStop + reEntryTimeToTurn + maneuverTime;
};
