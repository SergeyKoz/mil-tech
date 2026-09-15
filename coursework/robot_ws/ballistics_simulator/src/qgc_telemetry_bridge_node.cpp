#include <chrono>
#include <cmath>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "ballistics_simulator/msg/telemetry.hpp"
#include "ballistics_simulator/state_qos.hpp"

// ЗАМІНІТЬ на ваш заголовочний файл та тип повідомлення
// #include "your_custom_msgs/msg/your_telemetry_msg.hpp"

namespace
{
  // constexpr auto kDroneConfigTopic = "/checker/drone_config";
  // constexpr auto kAmmoConfigTopic = "/checker/ammo_config";
  constexpr auto kTelemetryTopic = "/checker/telemetry";
  constexpr auto kMavrosPositionTopic = "/mavros/vision_pose/pose";
  // constexpr auto kTargetTopic = "/checker/target";
  // constexpr auto kControlCommandTopic = "/checker/control_command";
  // constexpr auto kDropCommandTopic = "/checker/drop_command"
}

class QgcTelemetryBridgeNode : public rclcpp::Node
{
public:
  QgcTelemetryBridgeNode() : Node("qgc_telemetry_bridge_node")
  {

    const auto state_qos = ballistics_simulator::make_state_qos();

    // Публікатор у MAVROS
    mavrosPublisher = this->create_publisher<geometry_msgs::msg::PoseStamped>(
        kMavrosPositionTopic, 10);

    // telemetrySubscription = this->create_subscription<ballistics_simulator::msg::Telemetry>(
    //     kTelementyTopic, 10,
    //     std::bind(&QgcTelemetryBridgeNode::telemetryCallback, this, std::placeholders::_1));

    telemetrySubscription = create_subscription<ballistics_simulator::msg::Telemetry>(
        kTelemetryTopic, state_qos, [this](const ballistics_simulator::msg::Telemetry &telemetry)
        { on_telemetry(telemetry); });

    // Підписник на ваш топік телеметрії (замініть тип YourTelemetryMsg)
    /*
    telemetry_sub_ = this->create_subscription<your_custom_msgs::msg::YourTelemetryMsg>(
      "/your_telemetry_topic", 10,
      std::bind(&TelemetryBridge::telemetryCallback, this, std::placeholders::_1));
    */

    RCLCPP_INFO(this->get_logger(), "Telemetry Bridge Node Started");
  }

private:
  // Приклад колбеку (адаптуйте під поля вашого повідомлення)

  // void telemetryCallback(const ballistics_simulator::msg::Telemetry::SharedPtr telemetry)
  void on_telemetry(const ballistics_simulator::msg::Telemetry &telemetry)
  {

    // auto originLocation = mavlinkConfig.qgsConfig.originLocation;
    // auto originLatitudeDeg = originLocation.latitude;
    // auto originLongitudeDeg = originLocation.longtitude;

    //     struct Location {
    //     float latitude;
    //     float longtitude;
    // };

    float originLatitudeDeg = 48.983498065911704F;
    float originLongitudeDeg = 37.82220625228142F;

    // LAT: "48.983498065911704"
    //   LON: "37.82220625228142"

    constexpr double earthRadiusMeters = 6378137.0;

    // const auto telemetry = dronePhysics->getTelemetry();

    auto altitudeMm = static_cast<int32_t>(telemetry.z * 1000);
    auto relativeAltitudeMm = altitudeMm;

    const double latitudeOffsetDeg = telemetry.y / earthRadiusMeters * 180.0 / M_PI;
    const double longitudeOffsetDeg =
        telemetry.x / (earthRadiusMeters * std::cos(originLatitudeDeg * M_PI / 180.0)) * 180.0 / M_PI;

    const auto latitude = static_cast<int32_t>((originLatitudeDeg + latitudeOffsetDeg) * 1e7);
    const auto longitude = static_cast<int32_t>((originLongitudeDeg + longitudeOffsetDeg) * 1e7);

    auto pose_msg = geometry_msgs::msg::PoseStamped();

    pose_msg.header.stamp = this->now();
    pose_msg.header.frame_id = "map"; // або "odom"
    pose_msg.pose.position.x = longitude;
    pose_msg.pose.position.y = latitude;
    pose_msg.pose.orientation.z = std::sin(telemetry.dir / 2.0);
    pose_msg.pose.orientation.w = std::cos(telemetry.dir / 2.0);
    mavrosPublisher->publish(pose_msg);

    RCLCPP_INFO(get_logger(),
                "on telemetry t=%d x,y,z=%.2f,%.2f,%.2f vx,vy,speed=%.2f,%.2f,%.2f dir=%.2f state=%d",
                telemetry.t_ms,
                telemetry.x,
                telemetry.y,
                telemetry.z,
                telemetry.vx,
                telemetry.vy,
                telemetry.speed,
                telemetry.dir,
                telemetry.state);

    // Передаємо координати
    // pose_msg.pose.position.x = msg->x;
    // pose_msg.pose.position.y = msg->y;
    // pose_msg.pose.position.z = 0.0; // Якщо висоти немає

    // // Напрямок (direction): передбачаємо, що він у радіанах (Yaw).
    // // Якщо у градусах, розкоментуйте наступний рядок:
    // // double yaw_rad = msg->direction * M_PI / 180.0;
    // double yaw_rad = msg->direction;

    // // Перетворення Yaw (Euler) у Кватерніон ZW
    // pose_msg.pose.orientation.x = 0.0;
    // pose_msg.pose.orientation.y = 0.0;
    // pose_msg.pose.orientation.z = std::sin(yaw_rad / 2.0);
    // pose_msg.pose.orientation.w = std::cos(yaw_rad / 2.0);

    // mavros_pub_->publish(pose_msg);
  }

  rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr mavrosPublisher;
  rclcpp::Subscription<ballistics_simulator::msg::Telemetry>::SharedPtr telemetrySubscription;
};

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<QgcTelemetryBridgeNode>());
  rclcpp::shutdown();
  return 0;
}