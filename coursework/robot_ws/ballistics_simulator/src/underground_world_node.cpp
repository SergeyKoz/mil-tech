#include <chrono>
#include <cstdint>
#include <deque>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"

#include "ballistics_simulator/msg/cell_observation.hpp"
#include "ballistics_simulator/msg/enemy_down.hpp"
#include "ballistics_simulator/msg/local_scan.hpp"
#include "ballistics_simulator/msg/move_command.hpp"
#include "ballistics_simulator/msg/robot_metrics.hpp"
#include "ballistics_simulator/msg/robot_result.hpp"
#include "ballistics_simulator/scenario_loader.hpp"
#include "ballistics_simulator/state_qos.hpp"
#include "ballistics_simulator/world_model.hpp"

namespace {

using ballistics_simulator::msg::CellObservation;
using ballistics_simulator::msg::EnemyDown;
using ballistics_simulator::msg::LocalScan;
using ballistics_simulator::msg::MoveCommand;
using ballistics_simulator::msg::RobotMetrics;
using ballistics_simulator::msg::RobotResult;

constexpr auto kScanTopic = "/robot/local_scan";
constexpr auto kMoveTopic = "/robot/cmd_move";
constexpr auto kMetricsTopic = "/robot/metrics";
constexpr auto kResultTopic = "/robot/result";
constexpr auto kEnemyDownTopic = "/payload/enemy_down";

CellObservation make_cell_msg(const ballistics_simulator::ObservedCell& cell)
{
    CellObservation msg;
    msg.x = cell.position.x;
    msg.y = cell.position.y;
    msg.cell_type = ballistics_simulator::cell_kind_to_string(cell.kind);
    msg.contact_id = cell.contact_id;
    return msg;
}

LocalScan make_scan_msg(const ballistics_simulator::LocalScanData& scan)
{
    LocalScan msg;
    msg.scenario_name = scan.scenario_name;
    msg.robot_x = scan.robot.x;
    msg.robot_y = scan.robot.y;

    msg.cells.reserve(scan.cells.size());
    for (const auto& cell : scan.cells) {
        msg.cells.push_back(make_cell_msg(cell));
    }

    return msg;
}

RobotMetrics make_metrics_msg(const ballistics_simulator::MetricsSnapshot& metrics)
{
    RobotMetrics msg;
    msg.scenario_name = metrics.scenario_name;
    msg.steps_taken = metrics.steps_taken;
    msg.invalid_moves = metrics.invalid_moves;
    msg.contacts_seen = metrics.contacts_seen;
    msg.contacts_down = metrics.contacts_down;
    msg.invalid_triggers = metrics.invalid_triggers;
    msg.duplicate_triggers = metrics.duplicate_triggers;
    msg.unique_cells_seen = metrics.unique_cells_seen;
    msg.map_coverage_percent = metrics.map_coverage_percent;
    return msg;
}

RobotResult make_result_msg(const ballistics_simulator::ResultSnapshot& result)
{
    RobotResult msg;
    msg.scenario_name = result.scenario_name;
    msg.mission_result = result.mission_result;
    msg.reason = result.reason;
    msg.steps_taken = result.steps_taken;
    msg.max_steps = result.max_steps;
    return msg;
}

}  // namespace

class UndergroundWorldNode final : public rclcpp::Node {
  public:
    UndergroundWorldNode()
        : Node("underground_world_node")
        , scenario_path_(declare_parameter<std::string>("scenario_path", ""))
        , move_commit_period_(read_move_commit_period())
        , world_(ballistics_simulator::load_scenario(scenario_path_))
    {
        const auto state_qos = ballistics_simulator::make_state_qos();
        const auto event_qos = rclcpp::QoS{10};

        scan_pub_ = create_publisher<LocalScan>(kScanTopic, state_qos);
        metrics_pub_ = create_publisher<RobotMetrics>(kMetricsTopic, state_qos);
        result_pub_ = create_publisher<RobotResult>(kResultTopic, state_qos);

        move_sub_ = create_subscription<MoveCommand>(kMoveTopic, event_qos, [this](const MoveCommand::SharedPtr msg) { on_move(*msg); });

        enemy_down_sub_ =
            create_subscription<EnemyDown>(kEnemyDownTopic, event_qos, [this](const EnemyDown::SharedPtr msg) { on_enemy_down(*msg); });

        move_commit_timer_ = create_wall_timer(move_commit_period_, [this]() { commit_next_move(); });
        initial_publish_timer_ = create_wall_timer(std::chrono::milliseconds{250}, [this]() {
            publish_state();
            initial_publish_timer_->cancel();
        });

        RCLCPP_INFO(get_logger(),
                    "loaded scenario=%s path=%s start=(%d,%d) contacts=%zu move_commit_period_ms=%ld",
                    world_.scenario().name.c_str(),
                    scenario_path_.c_str(),
                    world_.scenario().start.x,
                    world_.scenario().start.y,
                    world_.scenario().contacts.size(),
                    move_commit_period_.count());
    }

  private:
    std::chrono::milliseconds read_move_commit_period()
    {
        const auto period_ms = declare_parameter<int>("move_commit_period_ms", 50);
        return std::chrono::milliseconds{period_ms < 1 ? 1 : period_ms};
    }

    void on_move(const MoveCommand& msg)
    {
        pending_moves_.push_back(msg.direction);
        RCLCPP_INFO(
            get_logger(), "queued move direction=%u pending_moves=%zu", static_cast<unsigned>(msg.direction), pending_moves_.size());
    }

    void on_enemy_down(const EnemyDown& msg)
    {
        const auto outcome = world_.apply_enemy_down(msg.contact_id, {msg.x, msg.y});
        RCLCPP_INFO(get_logger(),
                    "enemy_down contact_id=%d accepted=%s reason=%s",
                    msg.contact_id,
                    outcome.accepted ? "true" : "false",
                    outcome.reason.c_str());
        publish_state();
    }

    void commit_next_move()
    {
        if (pending_moves_.empty()) {
            return;
        }

        if (world_.terminal()) {
            pending_moves_.clear();
            return;
        }

        const auto direction = pending_moves_.front();
        pending_moves_.pop_front();

        const auto moved = world_.apply_move(direction);
        RCLCPP_INFO(get_logger(),
                    "committed move direction=%u accepted=%s pending_moves=%zu",
                    static_cast<unsigned>(direction),
                    moved ? "true" : "false",
                    pending_moves_.size());
        publish_state();
    }

    void publish_state()
    {
        const auto state = world_.snapshot();
        scan_pub_->publish(make_scan_msg(state.scan));
        metrics_pub_->publish(make_metrics_msg(state.metrics));
        result_pub_->publish(make_result_msg(state.result));

        RCLCPP_INFO(get_logger(),
                    "published state scenario=%s robot=(%d,%d) result=%s",
                    state.scan.scenario_name.c_str(),
                    state.scan.robot.x,
                    state.scan.robot.y,
                    state.result.mission_result.c_str());
    }

    std::string scenario_path_;
    std::chrono::milliseconds move_commit_period_;
    ballistics_simulator::WorldModel world_;
    std::deque<std::uint8_t> pending_moves_;
    rclcpp::Publisher<LocalScan>::SharedPtr scan_pub_;
    rclcpp::Publisher<RobotMetrics>::SharedPtr metrics_pub_;
    rclcpp::Publisher<RobotResult>::SharedPtr result_pub_;
    rclcpp::Subscription<MoveCommand>::SharedPtr move_sub_;
    rclcpp::Subscription<EnemyDown>::SharedPtr enemy_down_sub_;
    rclcpp::TimerBase::SharedPtr move_commit_timer_;
    rclcpp::TimerBase::SharedPtr initial_publish_timer_;
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<UndergroundWorldNode>());
    rclcpp::shutdown();
    return 0;
}
