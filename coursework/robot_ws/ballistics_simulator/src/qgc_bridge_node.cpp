#include <chrono>
#include <cmath>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
// #include <geometry_msgs/msg/pose_with_covariance_stamped.hpp>
// #include "sensor_msgs/msg/nav_sat_fix.hpp"
// #include "sensor_msgs/msg/nav_sat_status.hpp"
// #include "mavros_msgs/msg/hil_gps.hpp"
#include "ballistics_simulator/msg/telemetry.hpp"
#include "ballistics_simulator/msg/target.hpp"
#include "ballistics_simulator/state_qos.hpp"
#include <geographic_msgs/msg/geo_point_stamped.hpp>
#include "ballistics_simulator/srv/start_trigger.hpp"
#include <mavros_msgs/msg/adsb_vehicle.hpp>
#include <mavros_msgs/msg/manual_control.hpp>
#include <mavros_msgs/msg/state.hpp>
#include <mavros_msgs/msg/gpsinput.hpp>
// #include <mavros_msgs/msg/position_target.hpp>
#include <mavros_msgs/msg/global_position_target.hpp>

namespace
{

  constexpr auto kTelemetryTopic = "/checker/telemetry";
  constexpr auto kTargetTopic = "/checker/target";
  // constexpr auto kMavrosPositionTopic = "/mavros/global_position/global";
  // constexpr auto kMavrosPositionTopic = "/mavros/hil/gps";
  constexpr auto kMavrosPositionTopic = "/mavros/vision_pose/pose";
  // constexpr auto kMavrosPositionTopic = "/mavros/mocap/pose";
  constexpr auto kTriggerService = "/start/trigger";
  constexpr auto kMavrosSetOrigin = "/mavros/global_position/set_gp_origin";
  constexpr auto kMavrosASDBTargets = "/mavros/adsb/send";
  constexpr auto kMavrosState = "/mavros/state";
}

class QgcBridgeNode : public rclcpp::Node
{
public:
  QgcBridgeNode() : Node("qgc_bridge_node"), originLatitude(declare_parameter<float>("origin_latitude")), originLongitude(declare_parameter<float>("origin_longitude"))
  {
    const auto state_qos = ballistics_simulator::make_state_qos();

    rclcpp::QoS qos_profile(10);
    qos_profile.reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE);
    qos_profile.durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL);

    // mavrosPublisher = this->create_publisher<sensor_msgs::msg::NavSatFix>(
    //     kMavrosPositionTopic, 10);
    // mavrosPublisher = this->create_publisher<mavros_msgs::msg::HilGPS>(
    //     kMavrosPositionTopic, 10);

    mavrosPublisher = this->create_publisher<geometry_msgs::msg::PoseStamped>(
        kMavrosPositionTopic, 10);

    originMavrosPublisher = this->create_publisher<geographic_msgs::msg::GeoPointStamped>(kMavrosSetOrigin, 10);
    targetMavrosPublisher = this->create_publisher<mavros_msgs::msg::ADSBVehicle>(kMavrosASDBTargets, 10);

    telemetrySubscription = create_subscription<ballistics_simulator::msg::Telemetry>(
        kTelemetryTopic, state_qos, [this](const ballistics_simulator::msg::Telemetry &telemetry)
        { on_telemetry(telemetry); });

    targetSubscription = create_subscription<ballistics_simulator::msg::Target>(
        kTargetTopic, state_qos, [this](const ballistics_simulator::msg::Target &target)
        { on_target(target); });

    stateSubscription = create_subscription<mavros_msgs::msg::State>(
        kMavrosState, state_qos, [this](const mavros_msgs::msg::State &state)
        { on_state(state); });

    auto timer_period = std::chrono::milliseconds(30);

    telemetryScheduler = this->create_wall_timer(
        timer_period,
        std::bind(&QgcBridgeNode::telemetry_scheduler_callback, this));

    startTriggerClient = create_client<ballistics_simulator::srv::StartTrigger>(kTriggerService);

    if (!startTriggerClient->wait_for_service(std::chrono::seconds(3)))
    {
      RCLCPP_ERROR(get_logger(), "service %s is not available", kTriggerService);
      rclcpp::shutdown();
    }

    publish_origin(10);

    RCLCPP_INFO(this->get_logger(), "QGS Bridge Node Started");
  }

private:
  void publish_origin(int seconds)
  {
    std::thread([this, seconds]()
                {
                  std::this_thread::sleep_for(std::chrono::seconds(seconds));

                  auto msg = geographic_msgs::msg::GeoPointStamped();
                  msg.header.stamp = this->now();
                  msg.header.frame_id = "map";
                  msg.position.latitude = originLatitude;
                  msg.position.longitude = originLongitude;
                  msg.position.altitude = 100.0;

                  RCLCPP_INFO(get_logger(), "Publish origin");

                  originMavrosPublisher->publish(msg); })
        .detach();
  }

  void on_telemetry(const ballistics_simulator::msg::Telemetry &telemetry)
  {
    if (!inited)
    {
      inited = true;

      // Запам'ятовуємо початкову точку відліку
      startX = telemetry.x;
      startY = telemetry.y;

      prevX = 0.F;
      prevY = 0.F;
    }

    droneTelemetry = telemetry;
  }

  void on_target(const ballistics_simulator::msg::Target &target)
  {
    constexpr double earthRadiusMeters = 6378137.0;

    const double latitudeOffsetDeg = target.y / earthRadiusMeters * 180.0 / M_PI;
    const double longitudeOffsetDeg =
        target.x / (earthRadiusMeters * std::cos(originLatitude * M_PI / 180.0)) * 180.0 / M_PI;

    const auto latitude = originLatitude + latitudeOffsetDeg;
    const auto longitude = originLongitude + longitudeOffsetDeg;

    auto targetMessage = mavros_msgs::msg::ADSBVehicle();

    targetMessage.header.stamp = this->now();
    targetMessage.icao_address = 1000 + target.id; // Унікальний ідентифікатор
    targetMessage.callsign = "TARGET_" + std::to_string(target.id);

    // Координати цілі
    targetMessage.latitude = latitude;
    targetMessage.longitude = longitude;
    targetMessage.altitude = 10.F;

    // 3. Параметри джерела та орієнтації
    targetMessage.altitude_type = 1;   // 1 = ALT_TYPE_GEOMETRIC
    targetMessage.emitter_type = 11;   // 14 = UAV (або 11 = Ground Vehicle)
    targetMessage.heading = 0;         // Курс (в градусах * 100)
    targetMessage.hor_velocity = 0.0f; // Швидкість (м/с)

    // 4. Важливо: tslc (час з останнього контакту) = 1 секунда
    targetMessage.tslc.sec = 1;
    targetMessage.tslc.nanosec = 0;

    targetMessage.flags = mavros_msgs::msg::ADSBVehicle::FLAG_VALID_COORDS |
                          mavros_msgs::msg::ADSBVehicle::FLAG_VALID_ALTITUDE |
                          mavros_msgs::msg::ADSBVehicle::FLAG_VALID_HEADING |
                          mavros_msgs::msg::ADSBVehicle::FLAG_VALID_VELOCITY |
                          mavros_msgs::msg::ADSBVehicle::FLAG_VALID_CALLSIGN;

    targetMavrosPublisher->publish(targetMessage);
  }

  void on_control(const mavros_msgs::msg::ManualControl msg)
  {
    RCLCPP_INFO(this->get_logger(), "Pitch: %.2f, Roll: %.2f, Throttle: %.2f, Yaw: %.2f",
                msg.x, msg.y, msg.z, msg.r);
  }

  void on_state(const mavros_msgs::msg::State msg)
  {
    std::string mode = msg.mode;
    bool isArmed = msg.armed;

    if (isArmed)
    {
      if (mode == "GUIDED" && !started)
      {
        auto request = std::make_shared<ballistics_simulator::srv::StartTrigger::Request>();

        startTriggerClient->async_send_request(
            request, [this](rclcpp::Client<ballistics_simulator::srv::StartTrigger>::SharedFuture future)
            {
              const auto response = future.get();
              started = response->started;
              RCLCPP_INFO(get_logger(), "started=%s", response->started ? "true" : "false"); });
      }
    }

    RCLCPP_INFO(this->get_logger(), "Current Mode: %s | Armed: %d", mode.c_str(), isArmed);
  }

  void telemetry_scheduler_callback()
  {
    float posX, posY, posZ;

    if (inited)
    {
      posX = droneTelemetry.x - startX;
      posY = droneTelemetry.y - startY;
      posZ = droneTelemetry.z;
    }
    else
    {
      posX = 0.0F;
      posY = 0.0F;
      posZ = 0.0F;
    }

    auto msg = geometry_msgs::msg::PoseStamped();

    msg.header.stamp = this->now();
    msg.header.frame_id = "base_link"; // odom map або "base_link"

    float step = 0.15F;
    float dx = prevX - posX;

    if (dx > step)
    {
      posX = prevX - step;
    }

    if (dx < -step)
    {
      posX = prevX + step;
    }

    float dy = prevY - posY;

    if (dy > step)
    {
      posY = prevY - step;
    }

    if (dy < -step)
    {
      posY = prevY + step;
    }

    // Координати в метрах відносно точки старту
    msg.pose.position.x = posX;
    msg.pose.position.y = posY;
    msg.pose.position.z = droneTelemetry.z;

    // Кватерніон орієнтації (без розвороту: Roll=0, Pitch=0, Yaw=0)
    msg.pose.orientation.x = 0.0;
    msg.pose.orientation.y = 0.0;

    double yaw = inited ? droneTelemetry.dir : 0.F;

    msg.pose.orientation.x = 0.0;
    msg.pose.orientation.y = 0.0;
    msg.pose.orientation.z = std::sin(yaw / 2.0);
    msg.pose.orientation.w = std::cos(yaw / 2.0);

    prevX = posX;
    prevY = posY;

    mavrosPublisher->publish(msg);
  }

  rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr mavrosPublisher;
  rclcpp::Publisher<geographic_msgs::msg::GeoPointStamped>::SharedPtr originMavrosPublisher;
  rclcpp::Publisher<mavros_msgs::msg::ADSBVehicle>::SharedPtr targetMavrosPublisher;
  // rclcpp::Publisher<sensor_msgs::msg::NavSatFix>::SharedPtr mavrosPublisher;
  // rclcpp::Publisher<mavros_msgs::msg::HilGPS>::SharedPtr mavrosPublisher;
  // rclcpp::Publisher<mavros_msgs::msg::GPSINPUT>::SharedPtr gpsPublisher;
  // rclcpp::Publisher<mavros_msgs::msg::GlobalPositionTarget>::SharedPtr gpsPublisher;
  rclcpp::Subscription<ballistics_simulator::msg::Telemetry>::SharedPtr telemetrySubscription;
  rclcpp::Subscription<ballistics_simulator::msg::Target>::SharedPtr targetSubscription;
  rclcpp::Subscription<mavros_msgs::msg::State>::SharedPtr stateSubscription;
  rclcpp::Client<ballistics_simulator::srv::StartTrigger>::SharedPtr startTriggerClient;
  ballistics_simulator::msg::Telemetry droneTelemetry;

  bool inited{false};
  bool started{false};
  float startX, startY, prevX, prevY, posX, posY;
  float originLatitude, originLongitude;

  rclcpp::TimerBase::SharedPtr telemetryScheduler;
};

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<QgcBridgeNode>());
  rclcpp::shutdown();
  return 0;
}