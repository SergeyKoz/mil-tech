#include <rclcpp/rclcpp.hpp>
#include "ballistics_simulator/srv/payload_trigger.hpp"

namespace {

constexpr auto kTriggerService = "/payload/trigger";

}  // namespace

class TriggerServiceNode final : public rclcpp::Node {
  public:
    TriggerServiceNode()
        : Node("trigger_service_node")
    {
        trigger_service = create_service<ballistics_simulator::srv::PayloadTrigger>(
            kTriggerService,
            [this](const std::shared_ptr<ballistics_simulator::srv::PayloadTrigger::Request> request,
                   std::shared_ptr<ballistics_simulator::srv::PayloadTrigger::Response> response) { on_trigger(request, response); });

        RCLCPP_INFO(get_logger(), "serving %s", kTriggerService);
    }

  private:
    void on_trigger(const std::shared_ptr<ballistics_simulator::srv::PayloadTrigger::Request>& request,
                    const std::shared_ptr<ballistics_simulator::srv::PayloadTrigger::Response>& response)
    {
        response->accepted = true;
        response->reason = "Accepted";
    }

    rclcpp::Service<ballistics_simulator::srv::PayloadTrigger>::SharedPtr trigger_service;
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<TriggerServiceNode>());
    rclcpp::shutdown();
    return 0;
}
