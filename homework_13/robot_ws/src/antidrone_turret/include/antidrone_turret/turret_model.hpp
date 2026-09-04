#pragma once

#include <cstdint>

namespace antidrone_turret {

enum class TurretModelTargetState : std::uint8_t { NONE, LOW_CONFIDENCE, LOCKED };
enum class TurretModelAction : std::uint8_t { IDLE, TRACK };
enum class TurretModelTrigger : std::uint8_t { SKIP, REQUESTED, RELOADING };
enum class TurretActuatorStatus : std::uint8_t { NONE, READY, RELOADING };

struct TurretState {
    TurretModelTargetState targetState;
    TurretModelAction action;
    TurretModelTrigger triggerState;
    float confidence;
    float distance;

    std::string targetStateLabel()
    {
        switch (targetState) {
            case TurretModelTargetState::NONE:
                return "NONE";
            case TurretModelTargetState::LOW_CONFIDENCE:
                return "LOW_CONFIDENCE";
            case TurretModelTargetState::LOCKED:
                return "LOCKED";
            default:
                return "UNKNOWN";
        }
    }

    std::string actionLabel()
    {
        switch (action) {
            case TurretModelAction::IDLE:
                return "IDLE";
            case TurretModelAction::TRACK:
                return "TRACK";
            default:
                return "UNKNOWN";
        }
    }

    std::string triggerStateLabel()
    {
        switch (triggerState) {
            case TurretModelTrigger::RELOADING:
                return "RELOADING";
            case TurretModelTrigger::REQUESTED:
                return "REQUESTED";
            case TurretModelTrigger::SKIP:
                return "SKIP";
            default:
                return "UNKNOWN";
        }
    }
};

struct Position {
    float x;
    float y;
};

struct PositionError {
    float error_x;
    float error_y;
};

struct TargetParameters {
    bool visible;
    Position position;
    float distance;
    float confidence;
};

class TurretModel {
  public:
    void init(float confidenceThreshold, float maxDistance)
    {
        this->confidenceThreshold = confidenceThreshold;
        this->maxDistance = maxDistance;

        turretState = {.targetState = TurretModelTargetState::NONE,
                       .action = TurretModelAction::IDLE,
                       .triggerState = TurretModelTrigger::SKIP,
                       .confidence = 0.0F,
                       .distance = 0.0F};
        positionError = {.error_x = 0.0F, .error_y = 0.0F};
    }

    void applyTarget(TargetParameters target)
    {
        positionError = {.error_x = 0.0F, .error_y = 0.0F};
        turretState.confidence = target.confidence;
        turretState.distance = target.distance;

        if (!target.visible) {
            turretState.targetState = TurretModelTargetState::NONE;
            turretState.action = TurretModelAction::IDLE;
            turretState.triggerState = TurretModelTrigger::SKIP;

            return;
        }

        if (target.confidence < confidenceThreshold) {
            turretState.targetState = TurretModelTargetState::LOW_CONFIDENCE;
            turretState.action = TurretModelAction::IDLE;
            turretState.triggerState = TurretModelTrigger::SKIP;

            return;
        }

        turretState.targetState = TurretModelTargetState::LOCKED;
        turretState.action = TurretModelAction::TRACK;
        positionError = {.error_x = target.position.x - 320.F, .error_y = 240.F - target.position.y};

        if (target.distance > maxDistance || actuatorStatus == TurretActuatorStatus::NONE) {
            turretState.triggerState = TurretModelTrigger::SKIP;

            return;
        }

        if (actuatorStatus == TurretActuatorStatus::RELOADING) {
            turretState.triggerState = TurretModelTrigger::RELOADING;

            return;
        }

        turretState.triggerState = TurretModelTrigger::REQUESTED;
    }

    TurretState getState() const { return turretState; }
    PositionError getTargetError() const { return positionError; }
    void setActuatorStatus(TurretActuatorStatus actuatorStatus) { this->actuatorStatus = actuatorStatus; }

  private:
    TurretState turretState{};
    PositionError positionError{};
    float confidenceThreshold{0.F};
    float maxDistance{0.F};
    TurretActuatorStatus actuatorStatus{TurretActuatorStatus::NONE};
};

}  // namespace antidrone_turret
