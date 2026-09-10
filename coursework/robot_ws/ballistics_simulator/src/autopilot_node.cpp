#include <rclcpp/logging.hpp>
#include <rclcpp/rclcpp.hpp>

class AutopilotNode final : public rclcpp::Node
{
public:
    AutopilotNode()
        : Node("autopilot_node")
    {
        const auto qos = rclcpp::QoS{10};

        const auto state_qos = ballistics_simulator::make_state_qos();
    }

    // ~AutopilotNode() override { uartListener.stop(); }

    // private:
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<AutopilotNode>());
    rclcpp::shutdown();
    return 0;
}
