#include <rclcpp/logging.hpp>
#include <rclcpp/rclcpp.hpp>
#include "ballistics_simulator/msg/drone_config.hpp"
#include "ballistics_simulator/msg/ammo_config.hpp"
#include "ballistics_simulator/msg/telemetry.hpp"
#include "ballistics_simulator/state_qos.hpp"
#include "ballistics_simulator/autopilot.hpp"
#include "ballistics_simulator/solvers/table_solver.hpp"
#include "ballistics_simulator/common.hpp"

namespace
{
    constexpr auto kDroneConfigTopic = "/robot/drone_config";
    constexpr auto kAmmoConfigTopic = "/robot/ammo_config";
    constexpr auto kTelemetryTopic = "/robot/telemetry";
} // namespace

class AutopilotNode final : public rclcpp::Node
{
public:
    AutopilotNode()
        : Node("autopilot_node"), autopilot(std::make_unique<ballistics_simulator::TableSolver>("/coursework/robot_ws/ballistics_simulator/config/ballistic_table.txt"))
    {
        const auto qos = rclcpp::QoS{10};

        const auto state_qos = ballistics_simulator::make_state_qos();

        autopilot.setLogger([this](const std::string &msg)
                            { RCLCPP_INFO(this->get_logger(), "%s", msg.c_str()); });

        droneConfigSubscription = create_subscription<ballistics_simulator::msg::DroneConfig>(
            kDroneConfigTopic, qos, [this](const ballistics_simulator::msg::DroneConfig &droneConfig)
            { on_drone_config(droneConfig); });
        ammoConfigSubscription = create_subscription<ballistics_simulator::msg::AmmoConfig>(
            kAmmoConfigTopic, qos, [this](const ballistics_simulator::msg::AmmoConfig &ammoConfig)
            { on_ammo_config(ammoConfig); });
        telemetrySubscription = create_subscription<ballistics_simulator::msg::Telemetry>(
            kTelemetryTopic, qos, [this](const ballistics_simulator::msg::Telemetry &telemetry)
            { on_telemetry(telemetry); });
    }

private:
    void on_drone_config(const ballistics_simulator::msg::DroneConfig &droneConfig)
    {
        RCLCPP_INFO(get_logger(),
                    "drone config attackSpeed=%.2f accelerationPath=%.2f angularSpeed=%.2f turnThreshold=%.2f timeStep=%.2f timeScale=%.2f",
                    droneConfig.attack_speed,
                    droneConfig.acceleration_path,
                    droneConfig.angular_speed,
                    droneConfig.turn_threshold,
                    droneConfig.time_step,
                    droneConfig.time_scale);

        autopilot.setConfig({.startPos = {},
                             .altitude = 0.F,
                             .initialDir = 0.F,
                             .attackSpeed = droneConfig.attack_speed,
                             .accelerationPath = droneConfig.acceleration_path,
                             .ammo = {},
                             .arrayTimeStep = 0.F,
                             .simTimeStep = droneConfig.time_step,
                             .targetTimeStep = 0.F,
                             .physicsTimeStep = 0.F,
                             .timeScale = droneConfig.time_scale,
                             .hitRadius = 0.F,
                             .angularSpeed = droneConfig.angular_speed,
                             .turnThreshold = droneConfig.turn_threshold});
    }

    void on_ammo_config(const ballistics_simulator::msg::AmmoConfig &ammoConfig)
    {
        autopilot.setAmmo({
            .name = ammoConfig.name,
            .mass = ammoConfig.mass,
            .drag = ammoConfig.drag,
            .lift = ammoConfig.lift,
            .hitRadius = ammoConfig.hit_radius,
            .targetCount = ammoConfig.targets,
        });

        // AmmoConfig
        RCLCPP_INFO(get_logger(),
                    "ammo_config name=%s m,d,l=%.2f,%.2f,%.2f hitRadius=%.2f targets=%d",
                    ammoConfig.name,
                    ammoConfig.mass,
                    ammoConfig.drag,
                    ammoConfig.lift,
                    ammoConfig.hit_radius,
                    ammoConfig.targets);
    }

    void on_telemetry(const ballistics_simulator::msg::Telemetry &telemetry)
    {
        autopilot.processTelemetry({.state = ballistics_simulator::STOPPED,
                                    .position = {telemetry.x, telemetry.y},
                                    .altitude = telemetry.z,
                                    .speed = {telemetry.vx, telemetry.vy},
                                    .direction = telemetry.dir,
                                    .timeSinceStart = static_cast<float>(telemetry.t_ms) / 1000.0F});

        RCLCPP_INFO(get_logger(),
                    "telemetry t=%d x,y,z=%.2f,%.2f,%.2f vx,vy,speed=%.2f,%.2f,%.2f dir=%.2f state=%d",
                    telemetry.t_ms,
                    telemetry.x,
                    telemetry.y,
                    telemetry.z,
                    telemetry.vx,
                    telemetry.vy,
                    telemetry.speed,
                    telemetry.dir,
                    telemetry.state);
    }

    rclcpp::Subscription<ballistics_simulator::msg::DroneConfig>::SharedPtr droneConfigSubscription;
    rclcpp::Subscription<ballistics_simulator::msg::AmmoConfig>::SharedPtr ammoConfigSubscription;
    rclcpp::Subscription<ballistics_simulator::msg::Telemetry>::SharedPtr telemetrySubscription;

    ballistics_simulator::Autopilot autopilot;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<AutopilotNode>());
    rclcpp::shutdown();
    return 0;
}
