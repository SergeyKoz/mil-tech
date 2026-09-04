#include <rclcpp/rclcpp.hpp>

#include "antidrone_turret/msg/servo_command.hpp"

constexpr auto kServoTopic = "/servo/cmd";

namespace antidrone_turret {

inline std::string command_label(const antidrone_turret::msg::ServoCommand& command)
{
    switch (command.direction) {
        case antidrone_turret::msg::ServoCommand::LEFT:
            return "LEFT";
        case antidrone_turret::msg::ServoCommand::CENTER:
            return "CENTER";
        case antidrone_turret::msg::ServoCommand::RIGHT:
            return "RIGHT";
        default:
            return "UNKNOWN";
    }
}

}  // namespace antidrone_turret

class YawServoDriverNode final : public rclcpp::Node {
  public:
    YawServoDriverNode()
        : Node("yaw_servo_driver_node")
    {
        commandSubscription = create_subscription<antidrone_turret::msg::ServoCommand>(
            kServoTopic, 10, [this](const antidrone_turret::msg::ServoCommand& command) { on_command(command); });
    }

  private:
    rclcpp::Subscription<antidrone_turret::msg::ServoCommand>::SharedPtr commandSubscription;

    void on_command(const antidrone_turret::msg::ServoCommand& command)
    {
        RCLCPP_INFO(get_logger(),
                    "received servo command %s target_x=%.1f error_x=%.1f",
                    antidrone_turret::command_label(command).c_str(),
                    command.target_x,
                    command.error_x);
    }
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<YawServoDriverNode>());
    rclcpp::shutdown();
    return 0;
}
