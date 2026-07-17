#include <rclcpp/rclcpp.hpp>

#include "antidrone_turret/msg/gimbal_command.hpp"

constexpr auto kGimbalTopic = "/gimbal/cmd";

// namespace antidrone_turret {
namespace {

inline std::string command_label(const antidrone_turret::msg::GimbalCommand& command)
{
    switch (command.direction) {
        case antidrone_turret::msg::GimbalCommand::DOWN:
            return "DOWN";
        case antidrone_turret::msg::GimbalCommand::CENTER:
            return "CENTER";
        case antidrone_turret::msg::GimbalCommand::UP:
            return "UP";
        default:
            return "UNKNOWN";
    }
}

}  // namespace

class GimbalDriverNode final : public rclcpp::Node {
  public:
    GimbalDriverNode()
        : Node("gimbal_driver_node")
    {
        commandSubscription = create_subscription<antidrone_turret::msg::GimbalCommand>(
            kGimbalTopic, 10, [this](const antidrone_turret::msg::GimbalCommand& command) { on_command(command); });
    }

  private:
    rclcpp::Subscription<antidrone_turret::msg::GimbalCommand>::SharedPtr commandSubscription;

    void on_command(const antidrone_turret::msg::GimbalCommand& command)
    {
        RCLCPP_INFO(get_logger(),
                    "received command %s target_y=%.1f error_y=%.1f",
                    command_label(command).c_str(),
                    command.target_y,
                    command.error_y);
    }
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<GimbalDriverNode>());
    rclcpp::shutdown();
    return 0;
}