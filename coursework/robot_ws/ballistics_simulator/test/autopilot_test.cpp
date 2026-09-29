#include <gtest/gtest.h>
#include <vector>
#include "ballistics_simulator/autopilot.hpp"
#include "ballistics_simulator/target_selector.hpp"
#include "interfaces/ballistics_solver_interface.hpp"
#include "interfaces/targets_provider_interface.hpp"

namespace
{
    class BallisticsSolver : public IBallisticsSolver
    {
    public:
        void init() override
        {
        }
        ballistics_simulator::DropParameters calcDropParameters(const ballistics_simulator::AmmoParams &ammo, float v0, float z0)
        {
            return {
                .time = 0.1F,
                .distance = 10.F};
        }
    };

    class TargetsProvider : public ITargetsProvider
    {
    public:
        auto isReady() -> bool override
        {
            return true;
        }
        auto getTargetsCount() -> int override
        {
            return 1;
        }
        auto getTarget(int index) -> ballistics_simulator::TargetTelemetry
        {
            return targetTelemetry;
        }

        auto setTargetTelemetry(ballistics_simulator::TargetTelemetry targetTelemetry) -> void
        {
            this->targetTelemetry = targetTelemetry;
        }
    private:
        ballistics_simulator::TargetTelemetry targetTelemetry {};
    };

    TEST(AutopilotTest, FullScenario)
    {
        auto solver = std::make_unique<BallisticsSolver>();
        auto targetsProvider = std::make_shared<TargetsProvider>();
        targetsProvider->setTargetTelemetry({
                .position = {.x = 20.F, .y = 0.F},
                .speed = {.x = 10.F, .y = 0.F}});

        ballistics_simulator::Autopilot autopilot(std::move(solver), targetsProvider);

        std::vector<std::string> messages;

        autopilot.setLogger([&messages](const std::string &msg)
                    { messages.push_back(msg);});

        autopilot.setCommandHandler([&messages](const ballistics_simulator::ControlCommand &command)
                            { messages.push_back("Command handled"); });

        ballistics_simulator::DroneTelemetry telemetry1{
            .state = ballistics_simulator::DroneStatus::MOVING, 
            .position = {.x = 0.F, .y = 0.F},
            .altitude = 100.F,
            .speed = {.x = 10.F, .y = 0.F},
            .direction = 0.F,
            .timeSinceStart = 0.f
        };

        autopilot.processTelemetry(telemetry1);
        EXPECT_EQ(messages.back(), "isConfigured 0");

        ballistics_simulator::DroneConfig config{
            .startPos = {.x = 0.F, .y = 0.F}, // початкова позиція (x, y)
            .altitude = 100.F, // висота
            .initialDir = 0.F, // початковий напрямок (рад)
            .attackSpeed = 10.F, // швидкість атаки (м/с)
            .accelerationPath = 10.F, // шлях розгону (м)
            .ammo {}, // обрані боєприпаси
            .arrayTimeStep = 1.F, // крок часу масиву цілей
            .simTimeStep = 1.F, // крок симуляції
            .targetTimeStep = 1.F,
            .physicsTimeStep = 1.F,
            .timeScale = 1.F,
            .hitRadius = 3.F, // радіус влучення
            .angularSpeed = 1.F, // кутова швидкість (рад/с)
            .turnThreshold = 3.F, // поріг повороту (рад)
        };
        autopilot.setConfig(config);

        ballistics_simulator::AmmoConfig ammoConfig{
            .name = "VOG-17",
            .mass = 1.F,
            .drag = 1.F,
            .lift = 0.F,
            .hitRadius = 3.F,
        };
        autopilot.setAmmo(ammoConfig);
        EXPECT_EQ(messages.back(), "Set ammo");

        ballistics_simulator::DroneTelemetry telemetry2{
            .state = ballistics_simulator::DroneStatus::MOVING, 
            .position = {.x = 10.F, .y = 0.F},
            .altitude = 100.F,
            .speed = {.x = 10.F, .y = 0.F},
            .direction = 0.F,
            .timeSinceStart = 0.f
        };

        autopilot.processTelemetry(telemetry2);
        EXPECT_EQ(*(messages.end() - 5), "isTargetsDefined yes"); 
        EXPECT_EQ(*(messages.end() - 4), "dropParams d: 10 t: 0.1");        
        EXPECT_EQ(*(messages.end() - 3), "SimStep: droneDirection: 0 target:0 target pos: 20,20 time: 1.1");
        EXPECT_EQ(*(messages.end() - 2), "Command: 1 prev state: 0 acc: 5 ang: 0 max: 10");
        EXPECT_EQ(messages.back(), "Command handled");

        targetsProvider->setTargetTelemetry({
            .position = {.x = 30.F, .y = 0.F},
            .speed = {.x = 10.F, .y = 0.F}});
        ballistics_simulator::DroneTelemetry telemetry3{
            .state = ballistics_simulator::DroneStatus::MOVING, 
            .position = {.x = 20.F, .y = 0.F},
            .altitude = 100.F,
            .speed = {.x = 10.F, .y = 0.F},
            .direction = 0.F,
            .timeSinceStart = 0.f
        };

        bool targetHit = false;

        try
        {
            autopilot.processTelemetry(telemetry3);
        }
        catch (const ballistics_simulator::TargetHit &e)
        {
            targetHit = std::stoi(e.what()) == 0;
        }

        EXPECT_EQ(*(messages.end() - 2), "Command handled");
        EXPECT_EQ(messages.back(), "SimStep: droneDirection: 0 target:0 target pos: 30,30 time: 0.1");
        EXPECT_TRUE(targetHit);
    }
} // namespace
