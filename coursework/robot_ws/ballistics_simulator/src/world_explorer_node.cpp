#include <rclcpp/logging.hpp>
#include <rclcpp/rclcpp.hpp>
#include "ballistics_simulator/msg/cell_observation.hpp"
#include "ballistics_simulator/msg/enemy_down.hpp"
#include "ballistics_simulator/msg/local_scan.hpp"
#include "ballistics_simulator/msg/move_command.hpp"
#include "ballistics_simulator/msg/student_status.hpp"
#include "ballistics_simulator/srv/payload_trigger.hpp"
#include "ballistics_simulator/state_qos.hpp"
#include "ballistics_simulator/world_explorer.hpp"

namespace {

constexpr auto kScanTopic = "/robot/local_scan";
constexpr auto kMoveTopic = "/robot/cmd_move";
constexpr auto kEnemyDownTopic = "/payload/enemy_down";
constexpr auto kStatusTopic = "/student/status";
constexpr auto kTriggerService = "/payload/trigger";

ballistics_simulator::CellEnvironment create_cell_environment(const ballistics_simulator::msg::LocalScan& localScan)
{
    std::array<ballistics_simulator::CellType, 9> cellTypes;

    std::transform(localScan.cells.begin(),
                   localScan.cells.begin() + 9,
                   cellTypes.begin(),
                   [](const ballistics_simulator::msg::CellObservation& item) {
                       return item.cell_type == "#" ? ballistics_simulator::CellType::Wall : ballistics_simulator::CellType::Free;
                   });

    return {.position = {.x = localScan.robot_x, .y = localScan.robot_y}, .cellTypes = cellTypes};
}

const ballistics_simulator::msg::CellObservation* get_contact_cell(const ballistics_simulator::msg::LocalScan& localScan)
{
    for (auto& cell : localScan.cells) {
        if (cell.cell_type == "C") {
            return &cell;
        }
    }

    return nullptr;
}

const ballistics_simulator::msg::LocalScan reset_contact_cell(ballistics_simulator::msg::LocalScan localScan,
                                                              const ballistics_simulator::msg::CellObservation& contactCell)
{
    for (auto& cell : localScan.cells) {
        if (cell.x == contactCell.x && cell.y == contactCell.y) {
            cell.cell_type = "x";
        }
    }

    return localScan;
}

ballistics_simulator::msg::EnemyDown create_enemy_down(const ballistics_simulator::msg::CellObservation& contactCell)
{
    ballistics_simulator::msg::EnemyDown enemyDown;
    enemyDown.contact_id = contactCell.contact_id;
    enemyDown.x = contactCell.x;
    enemyDown.y = contactCell.y;

    return enemyDown;
}

ballistics_simulator::msg::MoveCommand create_move(const ballistics_simulator::MoveDirection& move)
{
    ballistics_simulator::msg::MoveCommand command;

    switch (move) {
        case ballistics_simulator::MoveDirection::Up:
            command.direction = ballistics_simulator::msg::MoveCommand::UP;

            break;
        case ballistics_simulator::MoveDirection::Down:
            command.direction = ballistics_simulator::msg::MoveCommand::DOWN;

            break;
        case ballistics_simulator::MoveDirection::Left:
            command.direction = ballistics_simulator::msg::MoveCommand::LEFT;

            break;
        case ballistics_simulator::MoveDirection::Right:
            command.direction = ballistics_simulator::msg::MoveCommand::RIGHT;

            break;
        default:
            throw std::runtime_error("Unknown command");
    }

    return command;
}

std::string moveLabel(ballistics_simulator::MoveDirection move)
{
    switch (move) {
        case ballistics_simulator::MoveDirection::Up:
            return "Up";
        case ballistics_simulator::MoveDirection::Down:
            return "Down";
        case ballistics_simulator::MoveDirection::Left:
            return "Left";
        case ballistics_simulator::MoveDirection::Right:
            return "Right";
        case ballistics_simulator::MoveDirection::Stop:
            return "Stop";
        default:
            return "UNKNOWN";
    }
}

enum class WorldExplorerState : std::uint8_t { EXPLORING, ENGAGING, RETURNING, FAILED, DONE };

ballistics_simulator::msg::StudentStatus create_state(const WorldExplorerState& state)
{
    ballistics_simulator::msg::StudentStatus status;

    switch (state) {
        case WorldExplorerState::EXPLORING:
            status.state = ballistics_simulator::msg::StudentStatus::EXPLORING;

            break;
        case WorldExplorerState::ENGAGING:
            status.state = ballistics_simulator::msg::StudentStatus::ENGAGING;

            break;
        case WorldExplorerState::RETURNING:
            status.state = ballistics_simulator::msg::StudentStatus::RETURNING;

            break;
        case WorldExplorerState::DONE:
            status.state = ballistics_simulator::msg::StudentStatus::DONE;

            break;
        case WorldExplorerState::FAILED:
            status.state = ballistics_simulator::msg::StudentStatus::FAILED;

            break;
        default:
            status.state = ballistics_simulator::msg::StudentStatus::FAILED;
    }

    return status;
}

std::string state_label(const WorldExplorerState& state)
{
    ballistics_simulator::msg::StudentStatus status;

    switch (state) {
        case WorldExplorerState::EXPLORING:
            return "EXPLORING";
        case WorldExplorerState::ENGAGING:
            return "ENGAGING";
        case WorldExplorerState::RETURNING:
            return "RETURNING";
        case WorldExplorerState::DONE:
            return "DONE";
        case WorldExplorerState::FAILED:
            return "FAILED";
        default:
            return "UNKNOWN";
    }
}

}  // namespace

class WorldExplorerNode final : public rclcpp::Node {
  public:
    WorldExplorerNode()
        : Node("world_explorer_node")
    {
        const auto qos = rclcpp::QoS{10};

        const auto state_qos = ballistics_simulator::make_state_qos();

        localScanSubscription = create_subscription<ballistics_simulator::msg::LocalScan>(
            kScanTopic, qos, [this](const ballistics_simulator::msg::LocalScan& localScan) { on_local_scan(localScan); });

        movePublicher = create_publisher<ballistics_simulator::msg::MoveCommand>(kMoveTopic, state_qos);
        enemyDownPublicher = create_publisher<ballistics_simulator::msg::EnemyDown>(kEnemyDownTopic, state_qos);
        statePublicher = create_publisher<ballistics_simulator::msg::StudentStatus>(kStatusTopic, state_qos);

        triggerClient = create_client<ballistics_simulator::srv::PayloadTrigger>(kTriggerService);

        if (!triggerClient->wait_for_service(std::chrono::seconds(3))) {
            RCLCPP_ERROR(get_logger(), "service %s is not available", kTriggerService);
            rclcpp::shutdown();
        }
    }

  private:
    void on_local_scan(const ballistics_simulator::msg::LocalScan& localScan)
    {
        RCLCPP_INFO(get_logger(),
                    "received local scan scenario=%s x=%d y=%d",
                    localScan.scenario_name.c_str(),
                    localScan.robot_x,
                    localScan.robot_y);

        auto current = worldExplorer.getCurrent();

        if (current == nullptr) {
            RCLCPP_INFO(get_logger(), "init worldExplorer");
            auto startCell = ballistics_simulator::Cell{.position = {.x = localScan.cells.at(4).x, .y = localScan.cells.at(4).y},
                                                        .type = ballistics_simulator::CellType::Free,
                                                        .cells = {}};
            worldExplorer.init(startCell);
        }

        auto move = ballistics_simulator::MoveDirection::Stop;

        auto* contactCell = get_contact_cell(localScan);

        if (contactCell != nullptr) {
            // a contact defined
            state = WorldExplorerState::ENGAGING;

            enemyDownPublicher->publish(create_enemy_down(*contactCell));

            auto request = std::make_shared<ballistics_simulator::srv::PayloadTrigger::Request>();
            request->contact_id = contactCell->contact_id;
            request->x = contactCell->x;
            request->y = contactCell->y;

            triggerClient->async_send_request(
                request, [this, localScan, contactCell](rclcpp::Client<ballistics_simulator::srv::PayloadTrigger>::SharedFuture future) {
                    const auto response = future.get();
                    RCLCPP_INFO(get_logger(), "accepted=%s reason=%s", response->accepted ? "true" : "false", response->reason.c_str());

                    if (response->accepted) {
                        RCLCPP_INFO(get_logger(),
                                    "push command:%s state:%s",
                                    moveLabel(ballistics_simulator::MoveDirection::Stop).c_str(),
                                    state_label(state).c_str());
                        statePublicher->publish(create_state(state));
                    }
                });

            state = WorldExplorerState::EXPLORING;

            return;
        }

        try {
            if (state == WorldExplorerState::EXPLORING) {
                worldExplorer.applyEnvironment(create_cell_environment(localScan));

                move = worldExplorer.nextStep();

                if (move == ballistics_simulator::MoveDirection::Stop) {
                    returnMovements = worldExplorer.goToStart();
                    state = WorldExplorerState::RETURNING;
                    RCLCPP_INFO(get_logger(), "return sequence length:%d", static_cast<int>(returnMovements.size()));
                }
                else {
                    worldExplorer.move(move);
                }
            }

            if (state == WorldExplorerState::RETURNING) {
                if (!returnMovements.empty()) {
                    move = returnMovements.front();
                    worldExplorer.move(move);
                    returnMovements.pop();
                }
                else {
                    state = WorldExplorerState::DONE;
                }
            }
        }
        catch (const std::runtime_error& ex) {
            state = WorldExplorerState::FAILED;
            RCLCPP_ERROR(get_logger(), "world exploring error. Error:%s", ex.what());
        }

        RCLCPP_INFO(get_logger(), "push command:%s state:%s", moveLabel(move).c_str(), state_label(state).c_str());
        statePublicher->publish(create_state(state));

        if (move != ballistics_simulator::MoveDirection::Stop) {
            movePublicher->publish(create_move(move));
        }
    }

    rclcpp::Subscription<ballistics_simulator::msg::LocalScan>::SharedPtr localScanSubscription;
    rclcpp::Publisher<ballistics_simulator::msg::MoveCommand>::SharedPtr movePublicher;
    rclcpp::Publisher<ballistics_simulator::msg::EnemyDown>::SharedPtr enemyDownPublicher;
    rclcpp::Publisher<ballistics_simulator::msg::StudentStatus>::SharedPtr statePublicher;
    rclcpp::Client<ballistics_simulator::srv::PayloadTrigger>::SharedPtr triggerClient;

    ballistics_simulator::WorldExplorer worldExplorer;
    WorldExplorerState state = WorldExplorerState::EXPLORING;
    std::queue<ballistics_simulator::MoveDirection> returnMovements;
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<WorldExplorerNode>());
    rclcpp::shutdown();
    return 0;
}
