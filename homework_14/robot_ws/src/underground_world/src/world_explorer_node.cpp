#include <rclcpp/logging.hpp>
#include <rclcpp/rclcpp.hpp>
#include "underground_world/msg/cell_observation.hpp"
#include "underground_world/msg/enemy_down.hpp"
#include "underground_world/msg/local_scan.hpp"
#include "underground_world/msg/move_command.hpp"
#include "underground_world/msg/student_status.hpp"
#include "underground_world/srv/payload_trigger.hpp"
#include "underground_world/state_qos.hpp"
#include "underground_world/world_explorer.hpp"

namespace {

constexpr auto kScanTopic = "/robot/local_scan";
constexpr auto kMoveTopic = "/robot/cmd_move";
constexpr auto kEnemyDownTopic = "/payload/enemy_down";
constexpr auto kStatusTopic = "/student/status";
constexpr auto kTriggerService = "/payload/trigger";

underground_world::CellEnvironment create_cell_environment(const underground_world::msg::LocalScan& localScan)
{
    std::array<underground_world::CellType, 9> cellTypes;

    std::transform(
        localScan.cells.begin(), localScan.cells.begin() + 9, cellTypes.begin(), [](const underground_world::msg::CellObservation& item) {
            return item.cell_type == "#" ? underground_world::CellType::Wall : underground_world::CellType::Free;
        });

    return {.position = {.x = localScan.robot_x, .y = localScan.robot_y}, .cellTypes = cellTypes};
}

const underground_world::msg::CellObservation* get_contact_cell(const underground_world::msg::LocalScan& localScan)
{
    for (auto& cell : localScan.cells) {
        if (cell.cell_type == "C") {
            return &cell;
        }
    }

    return nullptr;
}

const underground_world::msg::LocalScan reset_contact_cell(underground_world::msg::LocalScan localScan,
                                                           const underground_world::msg::CellObservation& contactCell)
{
    for (auto& cell : localScan.cells) {
        if (cell.x == contactCell.x && cell.y == contactCell.y) {
            cell.cell_type = "x";
        }
    }

    return localScan;
}

underground_world::msg::EnemyDown create_enemy_down(const underground_world::msg::CellObservation& contactCell)
{
    underground_world::msg::EnemyDown enemyDown;
    enemyDown.contact_id = contactCell.contact_id;
    enemyDown.x = contactCell.x;
    enemyDown.y = contactCell.y;

    return enemyDown;
}

underground_world::msg::MoveCommand create_move(const underground_world::MoveDirection& move)
{
    underground_world::msg::MoveCommand command;

    switch (move) {
        case underground_world::MoveDirection::Up:
            command.direction = underground_world::msg::MoveCommand::UP;

            break;
        case underground_world::MoveDirection::Down:
            command.direction = underground_world::msg::MoveCommand::DOWN;

            break;
        case underground_world::MoveDirection::Left:
            command.direction = underground_world::msg::MoveCommand::LEFT;

            break;
        case underground_world::MoveDirection::Right:
            command.direction = underground_world::msg::MoveCommand::RIGHT;

            break;
        default:
            throw std::runtime_error("Unknown command");
    }

    return command;
}

std::string moveLabel(underground_world::MoveDirection move)
{
    switch (move) {
        case underground_world::MoveDirection::Up:
            return "Up";
        case underground_world::MoveDirection::Down:
            return "Down";
        case underground_world::MoveDirection::Left:
            return "Left";
        case underground_world::MoveDirection::Right:
            return "Right";
        case underground_world::MoveDirection::Stop:
            return "Stop";
        default:
            return "UNKNOWN";
    }
}

enum class WorldExplorerState : std::uint8_t { EXPLORING, ENGAGING, RETURNING, FAILED, DONE };

underground_world::msg::StudentStatus create_state(const WorldExplorerState& state)
{
    underground_world::msg::StudentStatus status;

    switch (state) {
        case WorldExplorerState::EXPLORING:
            status.state = underground_world::msg::StudentStatus::EXPLORING;

            break;
        case WorldExplorerState::ENGAGING:
            status.state = underground_world::msg::StudentStatus::ENGAGING;

            break;
        case WorldExplorerState::RETURNING:
            status.state = underground_world::msg::StudentStatus::RETURNING;

            break;
        case WorldExplorerState::DONE:
            status.state = underground_world::msg::StudentStatus::DONE;

            break;
        case WorldExplorerState::FAILED:
            status.state = underground_world::msg::StudentStatus::FAILED;

            break;
        default:
            status.state = underground_world::msg::StudentStatus::FAILED;
    }

    return status;
}

std::string state_label(const WorldExplorerState& state)
{
    underground_world::msg::StudentStatus status;

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

        const auto state_qos = underground_world::make_state_qos();

        localScanSubscription = create_subscription<underground_world::msg::LocalScan>(
            kScanTopic, qos, [this](const underground_world::msg::LocalScan& localScan) { on_local_scan(localScan); });

        movePublicher = create_publisher<underground_world::msg::MoveCommand>(kMoveTopic, state_qos);
        enemyDownPublicher = create_publisher<underground_world::msg::EnemyDown>(kEnemyDownTopic, state_qos);
        statePublicher = create_publisher<underground_world::msg::StudentStatus>(kStatusTopic, state_qos);

        triggerClient = create_client<underground_world::srv::PayloadTrigger>(kTriggerService);

        if (!triggerClient->wait_for_service(std::chrono::seconds(3))) {
            RCLCPP_ERROR(get_logger(), "service %s is not available", kTriggerService);
            rclcpp::shutdown();
        }
    }

  private:
    void on_local_scan(const underground_world::msg::LocalScan& localScan)
    {
        RCLCPP_INFO(get_logger(),
                    "received local scan scenario=%s x=%d y=%d",
                    localScan.scenario_name.c_str(),
                    localScan.robot_x,
                    localScan.robot_y);

        auto current = worldExplorer.getCurrent();

        if (current == nullptr) {
            RCLCPP_INFO(get_logger(), "init worldExplorer");
            auto startCell = underground_world::Cell{.position = {.x = localScan.cells.at(4).x, .y = localScan.cells.at(4).y},
                                                     .type = underground_world::CellType::Free,
                                                     .cells = {}};
            worldExplorer.init(startCell);
        }

        auto move = underground_world::MoveDirection::Stop;

        auto* contactCell = get_contact_cell(localScan);

        if (contactCell != nullptr) {
            // a contact defined
            state = WorldExplorerState::ENGAGING;

            enemyDownPublicher->publish(create_enemy_down(*contactCell));

            auto request = std::make_shared<underground_world::srv::PayloadTrigger::Request>();
            request->contact_id = contactCell->contact_id;
            request->x = contactCell->x;
            request->y = contactCell->y;

            triggerClient->async_send_request(
                request, [this, localScan, contactCell](rclcpp::Client<underground_world::srv::PayloadTrigger>::SharedFuture future) {
                    const auto response = future.get();
                    RCLCPP_INFO(get_logger(), "accepted=%s reason=%s", response->accepted ? "true" : "false", response->reason.c_str());

                    if (response->accepted) {
                        RCLCPP_INFO(get_logger(),
                                    "push command:%s state:%s",
                                    moveLabel(underground_world::MoveDirection::Stop).c_str(),
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

                if (move == underground_world::MoveDirection::Stop) {
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

        if (move != underground_world::MoveDirection::Stop) {
            movePublicher->publish(create_move(move));
        }
    }

    rclcpp::Subscription<underground_world::msg::LocalScan>::SharedPtr localScanSubscription;
    rclcpp::Publisher<underground_world::msg::MoveCommand>::SharedPtr movePublicher;
    rclcpp::Publisher<underground_world::msg::EnemyDown>::SharedPtr enemyDownPublicher;
    rclcpp::Publisher<underground_world::msg::StudentStatus>::SharedPtr statePublicher;
    rclcpp::Client<underground_world::srv::PayloadTrigger>::SharedPtr triggerClient;

    underground_world::WorldExplorer worldExplorer;
    WorldExplorerState state = WorldExplorerState::EXPLORING;
    std::queue<underground_world::MoveDirection> returnMovements;
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<WorldExplorerNode>());
    rclcpp::shutdown();
    return 0;
}
