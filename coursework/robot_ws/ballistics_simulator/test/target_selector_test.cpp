#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include "ballistics_simulator/common.hpp"
#include "ballistics_simulator/target_selector.hpp"
#include "interfaces/targets_provider_interface.hpp"

class TargetsProvider : public ITargetsProvider
{
public:
    int count = 0;
    std::vector<ballistics_simulator::TargetTelemetry> targetsTelemetry{};

    bool isReady() override  { return true; };

    int getTargetsCount() override { return count; }

    ballistics_simulator::TargetTelemetry getTarget(int index) override
    {
        return targetsTelemetry.at(index);
    }
};

class TargetSelectorTest : public ::testing::Test
{
protected:
    std::shared_ptr<TargetsProvider> targetsProvider;
    std::unique_ptr<ballistics_simulator::TargetSelector> selector;
    ballistics_simulator::DroneConfig droneConfig;

    void SetUp() override
    {
        targetsProvider = std::make_shared<TargetsProvider>();
        selector = std::make_unique<ballistics_simulator::TargetSelector>(targetsProvider);

        // Налаштування базової конфігурації дрона
        droneConfig.turnThreshold = 0.1f;
        droneConfig.angularSpeed = 1.0f; // 1 рад/с
        droneConfig.attackSpeed = 10.0f; // 10 м/с
        droneConfig.accelerationPath = 10.0f;
        selector->init(droneConfig);
    }
};

TEST_F(TargetSelectorTest, SingleTargetDroneDeceleratingTurn)
{
    targetsProvider->count = 1;
    targetsProvider->targetsTelemetry.push_back({.position = {30.0F, 0.0F}, .speed{10.0F, 0.0F}});

    ballistics_simulator::DroneTelemetry droneTelemetry{};
    droneTelemetry.state = ballistics_simulator::DroneStatus::DECELERATING;
    droneTelemetry.position = {0.0F, 0.0F};
    droneTelemetry.speed = {0.0F, 5.0F};
    droneTelemetry.altitude = 100.0F;
    droneTelemetry.direction = 2.0F;
    droneTelemetry.timeSinceStart = 0.0F;
    ballistics_simulator::DropParameters dropParams = {.time = 5.0f, .distance = 10.0f};

    ballistics_simulator::SelectedTarget selected = selector->selectTarget(droneTelemetry, dropParams);

    EXPECT_EQ(selected.index, 0);
    EXPECT_NEAR(selected.timeToReachPosition, 10.75F, 0.001F);
}

TEST_F(TargetSelectorTest, SingleTargetDroneDeceleratingNoTurn)
{
    targetsProvider->count = 1;
    targetsProvider->targetsTelemetry.push_back({.position = {30.0F, 0.0F}, .speed{10.0F, 0.0F}});

    ballistics_simulator::DroneTelemetry droneTelemetry{};
    droneTelemetry.state = ballistics_simulator::DroneStatus::DECELERATING;
    droneTelemetry.position = {0.0F, 0.0F};
    droneTelemetry.speed = {5.0F, 0.0F};
    droneTelemetry.altitude = 100.0F;
    droneTelemetry.direction = 0.0F;
    droneTelemetry.timeSinceStart = 0.0F;
    ballistics_simulator::DropParameters dropParams = {.time = 5.0f, .distance = 10.0f};

    ballistics_simulator::SelectedTarget selected = selector->selectTarget(droneTelemetry, dropParams);

    EXPECT_EQ(selected.index, 0);
    EXPECT_NEAR(selected.timeToReachPosition, 7.25F, 0.001F);
}

TEST_F(TargetSelectorTest, SingleTargetDroneStopped)
{
    targetsProvider->count = 1;
    targetsProvider->targetsTelemetry.push_back({.position = {30.0F, 0.0F}, .speed{10.0F, 0.0F}});

    ballistics_simulator::DroneTelemetry droneTelemetry{};
    droneTelemetry.state = ballistics_simulator::DroneStatus::STOPPED;
    droneTelemetry.position = {0.0F, 0.0F};
    droneTelemetry.speed = {0.0F, 0.0F};
    droneTelemetry.altitude = 100.0F;
    droneTelemetry.direction = 0.0F;
    droneTelemetry.timeSinceStart = 0.0F;
    ballistics_simulator::DropParameters dropParams = {.time = 5.0f, .distance = 10.0f};

    ballistics_simulator::SelectedTarget selected = selector->selectTarget(droneTelemetry, dropParams);

    EXPECT_EQ(selected.index, 0);
    EXPECT_NEAR(selected.timeToReachPosition, 8.0F, 0.001F);
}

TEST_F(TargetSelectorTest, SingleTargetDroneMoving)
{
    targetsProvider->count = 1;
    targetsProvider->targetsTelemetry.push_back({.position = {30.0F, 0.0F}, .speed{10.0F, 0.0F}});

    ballistics_simulator::DroneTelemetry droneTelemetry{};
    droneTelemetry.state = ballistics_simulator::DroneStatus::MOVING;
    droneTelemetry.position = {0.0F, 0.0F};
    droneTelemetry.speed = {0.0F, 10.0F};
    droneTelemetry.altitude = 100.0F;
    droneTelemetry.direction = 1.5708F;
    droneTelemetry.timeSinceStart = 0.0F;
    ballistics_simulator::DropParameters dropParams = {.time = 5.0f, .distance = 10.0f};

    ballistics_simulator::SelectedTarget selected = selector->selectTarget(droneTelemetry, dropParams);

    EXPECT_EQ(selected.index, 0);
    EXPECT_NEAR(selected.timeToReachPosition, 10.57F, 0.001F);
}

TEST_F(TargetSelectorTest, SelectsFromTwoTargets)
{
    targetsProvider->count = 2;
    targetsProvider->targetsTelemetry.push_back({.position = {10.0F, 0.0F}, .speed{10.0F, 0.0F}});
    targetsProvider->targetsTelemetry.push_back({.position = {30.0F, 0.0F}, .speed{10.0F, 0.0F}});
    ballistics_simulator::DroneTelemetry droneTelemetry{};
    droneTelemetry.state = ballistics_simulator::DroneStatus::DECELERATING;
    droneTelemetry.position = {0.0F, 0.0F};
    droneTelemetry.speed = {0.0F, 5.0F};
    droneTelemetry.altitude = 100.0F;
    droneTelemetry.direction = 2.0F;
    droneTelemetry.timeSinceStart = 0.0F;
    ballistics_simulator::DropParameters dropParams = {.time = 5.0f, .distance = 10.0f};

    ballistics_simulator::SelectedTarget selected = selector->selectTarget(droneTelemetry, dropParams);

    EXPECT_EQ(selected.index, 1);
    EXPECT_NEAR(selected.timeToReachPosition, 10.75, 0.001);
}
