#include <rclcpp/rclcpp.hpp>
#include "ballistics_simulator/srv/drop_trigger.hpp"
#include "ballistics_simulator/msg/drop_command.hpp"
#include "ballistics_simulator/state_qos.hpp"

namespace
{

    constexpr auto kTriggerService = "/drop/trigger";
    constexpr auto kDropCommandTopic = "/checker/drop_command";

} // namespace

class DropServiceNode final : public rclcpp::Node
{
public:
    DropServiceNode()
        : Node("drop_service_node")
    {
        trigger_service = create_service<ballistics_simulator::srv::DropTrigger>(
            kTriggerService,
            [this](const std::shared_ptr<ballistics_simulator::srv::DropTrigger::Request> request,
                   std::shared_ptr<ballistics_simulator::srv::DropTrigger::Response> response)
            { on_trigger(request, response); });

        const auto state_qos = ballistics_simulator::make_state_qos();

        dropCommandPublisher = create_publisher<ballistics_simulator::msg::DropCommand>(kDropCommandTopic, state_qos);

        RCLCPP_INFO(get_logger(), "serving %s", kTriggerService);
    }

private:
    void on_trigger(const std::shared_ptr<ballistics_simulator::srv::DropTrigger::Request> &request,
                    const std::shared_ptr<ballistics_simulator::srv::DropTrigger::Response> &response)
    {
        ballistics_simulator::msg::DropCommand msg;
        msg.target_id = request->target_id;
        dropCommandPublisher->publish(msg);

        response->released = true;
    }

    rclcpp::Service<ballistics_simulator::srv::DropTrigger>::SharedPtr trigger_service;
    rclcpp::Publisher<ballistics_simulator::msg::DropCommand>::SharedPtr dropCommandPublisher;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<DropServiceNode>());
    rclcpp::shutdown();
    return 0;
}
