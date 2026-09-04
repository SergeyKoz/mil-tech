#include <rclcpp/rclcpp.hpp>

#include "antidrone_turret/target_formatter.hpp"
#include "antidrone_turret/turret_model.hpp"

#include "antidrone_turret/msg/target.hpp"
#include "antidrone_turret/msg/gimbal_command.hpp"
#include "antidrone_turret/msg/servo_command.hpp"
#include "antidrone_turret/msg/actuator_status.hpp"
#include "antidrone_turret/msg/turret_status.hpp"
#include "antidrone_turret/srv/trigger_actuator.hpp"

constexpr auto kTargetTopic = "/perception/target";
constexpr auto kGimbalTopic = "/gimbal/cmd";
constexpr auto kServoTopic = "/servo/cmd";
constexpr auto kTriggerService = "/actuator/trigger";
constexpr auto kActuatorStatusTopic = "/actuator/status";
constexpr auto kTurretStatusTopic = "/turret/status";

constexpr float epsilon = 1e-4f;

namespace {

inline std::string actuator_status_label(const antidrone_turret::msg::ActuatorStatus& actuatorStatus)
{
    switch (actuatorStatus.state) {
        case antidrone_turret::msg::ActuatorStatus::READY:
            return "READY";
        case antidrone_turret::msg::ActuatorStatus::RELOADING:
            return "RELOADING";
        default:
            return "UNKNOWN";
    }
}

inline int8_t calc_servo_direction(const float& errorX)
{
    if (std::abs(errorX) < epsilon) {
        return antidrone_turret::msg::ServoCommand::CENTER;
    }

    if (errorX < 0) {
        return antidrone_turret::msg::ServoCommand::LEFT;
    }

    return antidrone_turret::msg::ServoCommand::RIGHT;
}

inline int8_t calc_gimbal_direction(const float& errorY)
{
    if (std::abs(errorY) < epsilon) {
        return antidrone_turret::msg::GimbalCommand::CENTER;
    }

    if (errorY < 0) {
        return antidrone_turret::msg::GimbalCommand::DOWN;
    }

    return antidrone_turret::msg::GimbalCommand::UP;
}

inline antidrone_turret::msg::TurretStatus create_turret_status_from_model_state(const antidrone_turret::TurretState& turretState)
{
    auto turretStatus = antidrone_turret::msg::TurretStatus{};
    turretStatus.confidence = turretState.confidence;
    turretStatus.distance_m = turretState.distance;

    switch (turretState.targetState) {
        case antidrone_turret::TurretModelTargetState::LOCKED:
            turretStatus.target_state = antidrone_turret::msg::TurretStatus::TARGET_LOCKED;

            break;
        case antidrone_turret::TurretModelTargetState::LOW_CONFIDENCE:
            turretStatus.target_state = antidrone_turret::msg::TurretStatus::TARGET_LOW_CONFIDENCE;

            break;
        case antidrone_turret::TurretModelTargetState::NONE:
            turretStatus.target_state = antidrone_turret::msg::TurretStatus::TARGET_NONE;

            break;
        default:
            turretStatus.target_state = antidrone_turret::msg::TurretStatus::TARGET_NONE;
    }

    switch (turretState.action) {
        case antidrone_turret::TurretModelAction::IDLE:
            turretStatus.action = antidrone_turret::msg::TurretStatus::ACTION_IDLE;

            break;
        case antidrone_turret::TurretModelAction::TRACK:
            turretStatus.action = antidrone_turret::msg::TurretStatus::ACTION_TRACK;

            break;

        default:
            turretStatus.action = antidrone_turret::msg::TurretStatus::ACTION_IDLE;
    }

    switch (turretState.triggerState) {
        case antidrone_turret::TurretModelTrigger::SKIP:
            turretStatus.trigger_state = antidrone_turret::msg::TurretStatus::TRIGGER_SKIP;

            break;
        case antidrone_turret::TurretModelTrigger::RELOADING:
            turretStatus.trigger_state = antidrone_turret::msg::TurretStatus::TRIGGER_RELOADING;

            break;
        case antidrone_turret::TurretModelTrigger::REQUESTED:
            turretStatus.trigger_state = antidrone_turret::msg::TurretStatus::TRIGGER_REQUESTED;

            break;
        default:
            turretStatus.trigger_state = antidrone_turret::msg::TurretStatus::TRIGGER_SKIP;
    }

    return turretStatus;
}

}  // namespace

class TurretControllerNode final : public rclcpp::Node {
  public:
    TurretControllerNode()
        : Node("turret_controller_node")
    {
        const auto confidenceThreshold = declare_parameter<double>("confidence_threshold", 100.0F);
        const auto maxDistance = declare_parameter<double>("max_distance_m");
        RCLCPP_INFO(get_logger(), "turret_controller settings confidenceThreshold=%.1f maxDistance=%.1f", confidenceThreshold, maxDistance);
        turretModel.init(confidenceThreshold, maxDistance);

        targetSubscription = create_subscription<antidrone_turret::msg::Target>(
            kTargetTopic, 10, [this](const antidrone_turret::msg::Target& target) { on_target(target); });

        actuatorStatusSubscription = create_subscription<antidrone_turret::msg::ActuatorStatus>(
            kActuatorStatusTopic, 10, [this](const antidrone_turret::msg::ActuatorStatus& status) { on_actuator_status(status); });

        gimbalPublicher = create_publisher<antidrone_turret::msg::GimbalCommand>(kGimbalTopic, 10);
        servoPublicher = create_publisher<antidrone_turret::msg::ServoCommand>(kServoTopic, 10);
        turretStatusPublicher = create_publisher<antidrone_turret::msg::TurretStatus>(kTurretStatusTopic, 10);

        triggerActuatorClient = create_client<antidrone_turret::srv::TriggerActuator>(kTriggerService);

        if (!triggerActuatorClient->wait_for_service(std::chrono::seconds(3))) {
            RCLCPP_ERROR(get_logger(), "service %s is not available", kTriggerService);
            rclcpp::shutdown();
        }
    }

  private:
    void on_target(const antidrone_turret::msg::Target& target)
    {
        RCLCPP_INFO(get_logger(),
                    "received target label=%s x=%.1f y=%.1f distance_m=%.1f confidence=%.2f",
                    antidrone_turret::target_label(target).c_str(),
                    target.x,
                    target.y,
                    target.distance_m,
                    target.confidence);

        turretModel.applyTarget({
            .visible = target.visible,
            .position = {.x = target.x, .y = target.y},
            .distance = target.distance_m,
            .confidence = target.confidence,
        });

        auto turretState = turretModel.getState();

        RCLCPP_INFO(get_logger(),
                    "turretState target targetState=%s action=%s triggerState=%s distance=%.2f confidence=%.2f",
                    turretState.targetStateLabel().c_str(),
                    turretState.actionLabel().c_str(),
                    turretState.triggerStateLabel().c_str(),
                    turretState.distance,
                    turretState.confidence);

        if (turretState.action == antidrone_turret::TurretModelAction::TRACK) {
            RCLCPP_INFO(get_logger(), "TRACK servo & gimbal");
            auto targetError = turretModel.getTargetError();

            auto gimbalCommand = antidrone_turret::msg::GimbalCommand{};
            gimbalCommand.direction = calc_gimbal_direction(targetError.error_y);
            gimbalCommand.target_y = target.y;
            gimbalCommand.error_y = targetError.error_y;
            gimbalPublicher->publish(gimbalCommand);

            auto servoCommand = antidrone_turret::msg::ServoCommand{};
            servoCommand.direction = calc_servo_direction(targetError.error_x);
            servoCommand.target_x = target.x;
            servoCommand.error_x = targetError.error_x;
            servoPublicher->publish(servoCommand);

            turretStatusPublicher->publish(create_turret_status_from_model_state(turretState));

            if (turretState.triggerState == antidrone_turret::TurretModelTrigger::REQUESTED) {
                auto request = std::make_shared<antidrone_turret::srv::TriggerActuator::Request>();
                request->confidence = target.confidence;
                request->distance_m = target.distance_m;

                RCLCPP_INFO(get_logger(), "PULL trigger");

                triggerActuatorClient->async_send_request(
                    request, [this](rclcpp::Client<antidrone_turret::srv::TriggerActuator>::SharedFuture future) {
                        const auto response = future.get();
                        RCLCPP_INFO(
                            get_logger(), "accepted=%s trigger_count=%u", response->accepted ? "true" : "false", response->trigger_count);
                    });
            }
        }
    }

    void on_actuator_status(const antidrone_turret::msg::ActuatorStatus& actuatorStatus)
    {
        auto turretModelActuatorStatus = antidrone_turret::TurretActuatorStatus::NONE;

        switch (actuatorStatus.state) {
            case antidrone_turret::msg::ActuatorStatus::READY:
                turretModelActuatorStatus = antidrone_turret::TurretActuatorStatus::READY;
                break;
            case antidrone_turret::msg::ActuatorStatus::RELOADING:
                turretModelActuatorStatus = antidrone_turret::TurretActuatorStatus::RELOADING;
                break;
            default:
                break;
        }

        turretModel.setActuatorStatus(turretModelActuatorStatus);

        RCLCPP_INFO(get_logger(),
                    "received actuator_status label=%s trigger_count=%d",
                    actuator_status_label(actuatorStatus).c_str(),
                    actuatorStatus.trigger_count);
    }

    rclcpp::Subscription<antidrone_turret::msg::ActuatorStatus>::SharedPtr actuatorStatusSubscription;
    rclcpp::Subscription<antidrone_turret::msg::Target>::SharedPtr targetSubscription;
    rclcpp::Publisher<antidrone_turret::msg::GimbalCommand>::SharedPtr gimbalPublicher;
    rclcpp::Publisher<antidrone_turret::msg::ServoCommand>::SharedPtr servoPublicher;
    rclcpp::Publisher<antidrone_turret::msg::TurretStatus>::SharedPtr turretStatusPublicher;
    rclcpp::Client<antidrone_turret::srv::TriggerActuator>::SharedPtr triggerActuatorClient;
    antidrone_turret::TurretModel turretModel;
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<TurretControllerNode>());
    rclcpp::shutdown();
    return 0;
}
