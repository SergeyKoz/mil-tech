#include <rclcpp/rclcpp.hpp>
#include "ballistics_simulator/srv/start_trigger.hpp"
#include "ballistics_simulator/msg/start_command.hpp"
#include "ballistics_simulator/state_qos.hpp"

namespace
{

    constexpr auto kTriggerService = "/start/trigger";
    constexpr auto kStartCommandTopic = "/checker/start_command";

} // namespace

class StartServiceNode final : public rclcpp::Node
{
public:
    StartServiceNode()
        : Node("start_service_node")
    {
        triggerService = create_service<ballistics_simulator::srv::StartTrigger>(
            kTriggerService,
            [this](const std::shared_ptr<ballistics_simulator::srv::StartTrigger::Request> request,
                   std::shared_ptr<ballistics_simulator::srv::StartTrigger::Response> response)
            { on_trigger(request, response); });

        const auto state_qos = ballistics_simulator::make_state_qos();

        startCommandPublisher = create_publisher<ballistics_simulator::msg::StartCommand>(kStartCommandTopic, state_qos);

        RCLCPP_INFO(get_logger(), "serving %s", kTriggerService);
    }

private:
    void on_trigger(const std::shared_ptr<ballistics_simulator::srv::StartTrigger::Request> &request,
                    const std::shared_ptr<ballistics_simulator::srv::StartTrigger::Response> &response)
    {
        ballistics_simulator::msg::StartCommand msg;
        startCommandPublisher->publish(msg);

        response->started = true;
    }

    rclcpp::Service<ballistics_simulator::srv::StartTrigger>::SharedPtr triggerService;
    rclcpp::Publisher<ballistics_simulator::msg::StartCommand>::SharedPtr startCommandPublisher;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<StartServiceNode>());
    rclcpp::shutdown();
    return 0;
}
