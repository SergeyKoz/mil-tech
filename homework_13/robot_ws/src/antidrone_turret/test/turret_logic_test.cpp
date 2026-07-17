#include <gtest/gtest.h>
#include "antidrone_turret/turret_model.hpp"

namespace {

using TurretState = antidrone_turret::TurretState;
using TurretModel = antidrone_turret::TurretModel;
using TargetState = antidrone_turret::TurretModelTargetState;
using Action = antidrone_turret::TurretModelAction;
using Trigger = antidrone_turret::TurretModelTrigger;
using ActuatorStatus = antidrone_turret::TurretActuatorStatus;
using TargetError = antidrone_turret::PositionError;

void expect_equal_turret_state(TurretState expectedState, TurretState actualState)
{
    EXPECT_EQ(expectedState.targetState, actualState.targetState);
    EXPECT_EQ(expectedState.action, actualState.action);
    EXPECT_EQ(expectedState.triggerState, actualState.triggerState);
    EXPECT_FLOAT_EQ(expectedState.confidence, actualState.confidence);
    EXPECT_FLOAT_EQ(expectedState.distance, actualState.distance);
}

void expect_equal_target_error(TargetError expectedError, TargetError actualError)
{
    EXPECT_FLOAT_EQ(expectedError.error_x, actualError.error_x);
    EXPECT_FLOAT_EQ(expectedError.error_y, actualError.error_y);
}

TEST(TurretModelTest, DefaultState)
{
    auto expectedState = TurretState{TargetState::NONE, Action::IDLE, Trigger::SKIP, 0.0F, 0.0F};
    auto turret = TurretModel{};

    auto actualState = turret.getState();

    expect_equal_turret_state(expectedState, actualState);
}

TEST(TurretModelTest, InitedState)
{
    auto expectedState = TurretState{TargetState::NONE, Action::IDLE, Trigger::SKIP, 0.0F, 0.0F};

    auto turret = TurretModel{};
    turret.init(0.9F, 10.0F);
    turret.setActuatorStatus(ActuatorStatus::READY);

    auto actualState = turret.getState();

    expect_equal_turret_state(expectedState, actualState);
}

TEST(TurretModelTest, TargetInvisible)
{
    auto expectedState = TurretState{TargetState::NONE, Action::IDLE, Trigger::SKIP, 0.0F, 0.0F};
    auto expectedError = TargetError{0.0F, 0.0F};

    auto turret = TurretModel{};
    turret.init(0.9F, 10.0F);
    turret.setActuatorStatus(ActuatorStatus::READY);
    turret.applyTarget({false, {0.0F, 0.0F}, 0.0F, 0.0F});

    expect_equal_turret_state(expectedState, turret.getState());
    expect_equal_target_error(expectedError, turret.getTargetError());
}

TEST(TurretModelTest, TargetLowConfidence)
{
    auto expectedState = TurretState{TargetState::LOW_CONFIDENCE, Action::IDLE, Trigger::SKIP, 0.5F, 5.0F};
    auto expectedError = TargetError{0.0F, 0.0F};

    auto turret = TurretModel{};
    turret.init(0.9F, 10.0F);
    turret.setActuatorStatus(ActuatorStatus::READY);
    turret.applyTarget({true, {10.0F, 10.0F}, 5.0F, 0.5F});

    expect_equal_turret_state(expectedState, turret.getState());
    expect_equal_target_error(expectedError, turret.getTargetError());
}

TEST(TurretModelTest, TargetTracking)
{
    auto expectedState = TurretState{TargetState::LOCKED, Action::TRACK, Trigger::SKIP, 0.95F, 15.0F};
    auto expectedError = TargetError{-310.0F, 230.0F};

    auto turret = TurretModel{};
    turret.init(0.9F, 10.0F);
    turret.setActuatorStatus(ActuatorStatus::READY);
    turret.applyTarget({true, {10.0F, 10.0F}, 15.0F, 0.95F});

    expect_equal_turret_state(expectedState, turret.getState());
    expect_equal_target_error(expectedError, turret.getTargetError());
}

TEST(TurretModelTest, TriggerRequestedState)
{
    auto expectedState = TurretState{TargetState::LOCKED, Action::TRACK, Trigger::REQUESTED, 0.95F, 9.0F};
    auto expectedError = TargetError{-320.0F, 240.0F};

    auto turret = TurretModel{};
    turret.init(0.9F, 10.0F);
    turret.setActuatorStatus(ActuatorStatus::READY);
    turret.applyTarget({true, {0.0F, 0.0F}, 9.0F, 0.95F});

    expect_equal_turret_state(expectedState, turret.getState());
    expect_equal_target_error(expectedError, turret.getTargetError());
}

TEST(TurretModelTest, ActuatorReloading)
{
    auto expectedState = TurretState{TargetState::LOCKED, Action::TRACK, Trigger::RELOADING, 0.95F, 5.0F};
    auto expectedError = TargetError{-310.0F, 230.0F};

    auto turret = TurretModel{};
    turret.init(0.9F, 10.0F);
    turret.setActuatorStatus(ActuatorStatus::RELOADING);
    turret.applyTarget({true, {10.0F, 10.0F}, 5.0F, 0.95F});

    expect_equal_turret_state(expectedState, turret.getState());
    expect_equal_target_error(expectedError, turret.getTargetError());
}

TEST(TurretModelTest, ApproachScenario)
{
    auto turret = TurretModel{};
    turret.init(0.9F, 10.0F);
    turret.setActuatorStatus(ActuatorStatus::READY);

    // not visible
    turret.applyTarget({false, {0.0F, 0.0F}, 0.0F, 0.0F});
    expect_equal_turret_state({TargetState::NONE, Action::IDLE, Trigger::SKIP, 0.0F, 0.0F}, turret.getState());
    expect_equal_target_error({0.0F, 0.0F}, turret.getTargetError());

    // visible too far low confidence
    turret.applyTarget({true, {10.0F, 10.0F}, 20.0F, 0.5F});
    expect_equal_turret_state({TargetState::LOW_CONFIDENCE, Action::IDLE, Trigger::SKIP, 0.5F, 20.0F}, turret.getState());
    expect_equal_target_error({0.0F, 0.0F}, turret.getTargetError());

    // visible too far LOCKED
    turret.applyTarget({true, {10.0F, 10.0F}, 15.0F, 0.95F});
    expect_equal_turret_state({TargetState::LOCKED, Action::TRACK, Trigger::SKIP, 0.95F, 15.0F}, turret.getState());
    expect_equal_target_error({-310.0F, 230.0F}, turret.getTargetError());

    // requested
    turret.applyTarget({true, {400.0F, 300.0F}, 5.0F, 0.95F});
    expect_equal_turret_state({TargetState::LOCKED, Action::TRACK, Trigger::REQUESTED, 0.95F, 5.0F}, turret.getState());
    expect_equal_target_error({80.0F, -60.0F}, turret.getTargetError());

    // reloading
    turret.setActuatorStatus(ActuatorStatus::RELOADING);
    turret.applyTarget({true, {400.0F, 300.0F}, 1.0F, 0.99F});
    expect_equal_turret_state({TargetState::LOCKED, Action::TRACK, Trigger::RELOADING, 0.99F, 1.0F}, turret.getState());
    expect_equal_target_error({80.0F, -60.0F}, turret.getTargetError());
}

}  // namespace
