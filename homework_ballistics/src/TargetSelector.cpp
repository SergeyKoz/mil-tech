#include "TargetSelector.hpp"
#include "IConfigLoader.hpp"
#include "ITargetsProvider.hpp"

TargetSelector::TargetSelector(ITargetsProvider& targetProvider)
  : targetProvider(&targetProvider)
{
}

void TargetSelector::init(const DroneConfig& droneConfig)
{
    this->droneConfig = &droneConfig;
    acceleration = droneConfig.acceleration();
    fullAccelerationTime = droneConfig.fullAccelerationTime();
    angleStep = droneConfig.angleStep();
};

SelectedTarget TargetSelector::selectTarget(float currentTime, const SimStep& simulationStep, float dropDistance)
{
    float minTotalTime = 0.f;
    int selectedTargetIndex = -1;
    Target* selectedTarget = nullptr;
    Coord currentTargetPos;

    for (int targetIndex = 0; targetIndex < targetProvider->getTargetsCount() - 1; targetIndex++) {
        Target target = targetProvider->getTarget(targetIndex);
        currentTargetPos = target.interpolate(currentTime, droneConfig->arrayTimeStep);

        // define total time to reach target
        // totalTime time to stop + time to turn + time to accelerate + time to move
        float timeToTurn{0.f};

        // time to stop + time to turn + time to accelerate + time to move
        bool isNeedTurnAngle = false;
        float targetDirection = simulationStep.pos.direction(currentTargetPos);
        float turnAngle = targetDirection - simulationStep.direction;

        if (std::fabs(turnAngle) > droneConfig->turnThreshold) {
            timeToTurn = std::abs(turnAngle) / droneConfig->angularSpeed;
            isNeedTurnAngle = true;
        }

        // drop point calculation
        float distanceToTarget = currentTargetPos.distance(simulationStep.pos);
        float distanceToDropPoint = distanceToTarget - dropDistance;

        float reEntryPath = calcReEntryPath(distanceToDropPoint, isNeedTurnAngle, simulationStep.speed, acceleration, *droneConfig);

        float totalTime =
          reEntryPath > 0 ? calcReEntryTime(simulationStep.speed, reEntryPath, turnAngle, acceleration, fullAccelerationTime, *droneConfig)
                          : calcEntryTime(simulationStep.speed,
                                          simulationStep.state,
                                          distanceToDropPoint,
                                          isNeedTurnAngle,
                                          acceleration,
                                          fullAccelerationTime,
                                          *droneConfig) +
                              timeToTurn;

        if (totalTime < 0) {
            continue;
        }

        if (selectedTargetIndex == -1 || totalTime < minTotalTime) {
            minTotalTime = totalTime;
            selectedTarget = &target;
            selectedTargetIndex = targetIndex;
        }
    }

    return {.idx = selectedTargetIndex, .target = selectedTarget, .position = currentTargetPos, .timeToReachPosition = minTotalTime};
};

float TargetSelector::calcReEntryPath(
  float distanceToDropPoint, bool isNeedTurnAngle, float speed, float acceleration, const DroneConfig& droneConfig)
{
    // define reentry path
    float reEntryPath = distanceToDropPoint < 0 ? -distanceToDropPoint : 0.f;

    if (!isNeedTurnAngle) {
        if (speed < droneConfig.attackSpeed) {
            float accelerationPath = (droneConfig.attackSpeed * droneConfig.attackSpeed - speed * speed) / (2 * acceleration);

            if (distanceToDropPoint - accelerationPath < 0) {
                reEntryPath += accelerationPath;
            }
        }
    }
    else {
        float stopingPath = 0.f;

        if (speed > 0) {
            stopingPath = (speed * speed) / (2 * acceleration);
        }

        if (distanceToDropPoint < stopingPath + droneConfig.accelerationPath) {
            reEntryPath = reEntryPath + stopingPath + droneConfig.accelerationPath;
        }
    }

    return reEntryPath;
};

float TargetSelector::calcEntryTime(float speed,
                                    DroneStatus state,
                                    float distanceToDropPoint,
                                    bool isNeedTurnAngle,
                                    float acceleration,
                                    float fullAccelerationTime,
                                    const DroneConfig& droneConfig)
{
    float timeToStop{0.f};
    float timeToAccelerate{0.f};
    float timeToMove{0.f};

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

float TargetSelector::calcReEntryTime(
  float speed, float reEntryPath, float turnAngle, float acceleration, float fullAccelerationTime, const DroneConfig& droneConfig)
{
    float reEntryTimeToTurn = (2 * M_PI) / droneConfig.angularSpeed + turnAngle;  // 360
    float timeToStop = 0.f;

    if (speed > 0) {
        timeToStop = speed / acceleration;
    }

    float maneuverTime = fullAccelerationTime + fullAccelerationTime;

    if (reEntryPath > 2 * droneConfig.accelerationPath) {
        maneuverTime = maneuverTime + (reEntryPath - 2 * droneConfig.accelerationPath) / droneConfig.attackSpeed;
    }

    return timeToStop + reEntryTimeToTurn + maneuverTime;
};
