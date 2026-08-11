#include <rclcpp/rclcpp.hpp>
#include "underground_world/srv/payload_trigger.hpp"
#include "underground_world/msg/enemy_down.hpp"
#include "underground_world/state_qos.hpp"

namespace {

constexpr auto kTriggerService = "/payload/trigger";
constexpr auto kEnemyDownTopic = "/payload/enemy_down";

underground_world::msg::EnemyDown create_enemy_down(const std::shared_ptr<underground_world::srv::PayloadTrigger::Request>& request)
{
    underground_world::msg::EnemyDown enemyDown;
    enemyDown.contact_id = request->contact_id;
    enemyDown.x = request->x;
    enemyDown.y = request->y;

    return enemyDown;
}

}  // namespace

class TriggerServiceNode final : public rclcpp::Node {
  public:
    TriggerServiceNode()
        : Node("trigger_service_node")
    {
        const auto state_qos = underground_world::make_state_qos();

        trigger_service = create_service<underground_world::srv::PayloadTrigger>(
            kTriggerService,
            [this](const std::shared_ptr<underground_world::srv::PayloadTrigger::Request> request,
                   std::shared_ptr<underground_world::srv::PayloadTrigger::Response> response) { on_trigger(request, response); });
        enemyDownPublicher = create_publisher<underground_world::msg::EnemyDown>(kEnemyDownTopic, state_qos);

        RCLCPP_INFO(get_logger(), "serving %s", kTriggerService);
    }

  private:
    void on_trigger(const std::shared_ptr<underground_world::srv::PayloadTrigger::Request>& request,
                    const std::shared_ptr<underground_world::srv::PayloadTrigger::Response>& response)
    {
        enemyDownPublicher->publish(create_enemy_down(request));

        response->accepted = true;
        response->reason = "Accepted";
    }

    rclcpp::Service<underground_world::srv::PayloadTrigger>::SharedPtr trigger_service;
    rclcpp::Publisher<underground_world::msg::EnemyDown>::SharedPtr enemyDownPublicher;
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<TriggerServiceNode>());
    rclcpp::shutdown();
    return 0;
}
