#include <rclcpp/rclcpp.hpp>
#include "underground_world/srv/payload_trigger.hpp"

namespace {

constexpr auto kTriggerService = "/payload/trigger";

}  // namespace

class TriggerServiceNode final : public rclcpp::Node {
  public:
    TriggerServiceNode()
        : Node("trigger_service_node")
    {
        trigger_service = create_service<underground_world::srv::PayloadTrigger>(
            kTriggerService,
            [this](const std::shared_ptr<underground_world::srv::PayloadTrigger::Request> request,
                   std::shared_ptr<underground_world::srv::PayloadTrigger::Response> response) { on_trigger(request, response); });

        RCLCPP_INFO(get_logger(), "serving %s", kTriggerService);
    }

  private:
    void on_trigger(const std::shared_ptr<underground_world::srv::PayloadTrigger::Request>& request,
                    const std::shared_ptr<underground_world::srv::PayloadTrigger::Response>& response)
    {
        response->accepted = true;
        response->reason = "Accepted";
    }

    rclcpp::Service<underground_world::srv::PayloadTrigger>::SharedPtr trigger_service;
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<TriggerServiceNode>());
    rclcpp::shutdown();
    return 0;
}
