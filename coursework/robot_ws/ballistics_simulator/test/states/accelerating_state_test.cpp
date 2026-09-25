#include <gtest/gtest.h>
#include "ballistics_simulator/common.hpp"
#include "ballistics_simulator/states/accelerating_state.hpp"

namespace
{
    // Фікстура для налаштування спільних даних тесту
    class AcceleratingStateTest : public ::testing::Test
    {
    protected:
        ballistics_simulator::AcceleratingState state;
        ballistics_simulator::DroneConfig config{};
        ballistics_simulator::DroneTelemetry telemetry{};
        ballistics_simulator::SimStep step{};
        ballistics_simulator::DroneContext context{};

        void SetUp() override
        {
            // Базові дефолтні налаштування
            config.turnThreshold = 0.1f;
            config.attackSpeed = 10.0f;
            config.accelerationPath = 10.0f;

            context.droneConfig = &config;
            context.simulationStep = &step;
            context.acceleration = 5.0f;
            context.distanceToDropPoint = 90.0f;

            telemetry.speed = {10.0f, 0.0F};
            telemetry.direction = 0.0f;
            telemetry.position = {0.0f, 0.0f};
            step.predictedTarget = {100.0f, 0.0f}; // прямо перед дроном

            context.droneTelemetry = telemetry;
        }
    };

    // 1. Звичайний розгін (рух у правильному напрямку, швидкість менше attackSpeed)
    TEST_F(AcceleratingStateTest, AccelerateToAttackSpeed)
    {
        context.droneTelemetry.speed = {20.0f, 0.0F}; // менше за attackSpeed (50.0f)

        auto command = state.execute(context);

        EXPECT_EQ(command.state, ballistics_simulator::ACCELERATING);
        EXPECT_FLOAT_EQ(command.acceleration, context.acceleration);
        EXPECT_FLOAT_EQ(command.maxSpeed, config.attackSpeed);
        EXPECT_FLOAT_EQ(command.angleSpeed, 0.0f);
    }

    // 2. Перехід у стан MOVING при досягненні цільової швидкості (attackSpeed)
    TEST_F(AcceleratingStateTest, GetAttackSpeedReached)
    {
        context.droneTelemetry.speed = {config.attackSpeed, 0.0F}; // досягли attackSpeed

        auto command = state.execute(context);

        EXPECT_EQ(command.state, ballistics_simulator::MOVING);
        EXPECT_FLOAT_EQ(command.acceleration, 0.0f);
        EXPECT_FLOAT_EQ(command.maxSpeed, config.attackSpeed);
        EXPECT_FLOAT_EQ(command.angleSpeed, 0.0f);
    }

    // // 3. Перехід у DECELERATING, якщо потрібно розвернутися до цілі (кут > turnThreshold)
    TEST_F(AcceleratingStateTest, TurnIsRequired)
    {
        // Ціль знаходиться під кутом, який перевищує turnThreshold
        step.predictedTarget = {0.0f, 100.0f}; // поворот на ~90 градусів (1.57 рад)

        auto command = state.execute(context);

        EXPECT_EQ(command.state, ballistics_simulator::DECELERATING);
        EXPECT_FLOAT_EQ(command.acceleration, context.acceleration);
        EXPECT_FLOAT_EQ(command.maxSpeed, 0.0f);
        EXPECT_FLOAT_EQ(command.angleSpeed, 0.0f);
    }

    // // 4. Сценарій перельоту точки скиду (distanceToDropPoint < 0) - Потрібен розворот
    TEST_F(AcceleratingStateTest, NeedReenty)
    {
        context.distanceToDropPoint = -1.0f;
        // context.droneTelemetry.direction = 0.0f; // Летимо в напрямку 0
        step.predictedTarget = {9.0f, 0.0f}; 

        auto command = state.execute(context);

        // Оскільки напрямок не співпадає з обрахованим зворотним, маємо гальмувати для повороту
        EXPECT_EQ(command.state, ballistics_simulator::DECELERATING);
        EXPECT_FLOAT_EQ(command.acceleration, context.acceleration);
        EXPECT_FLOAT_EQ(command.maxSpeed, 0.0f);
    }
} // namespace