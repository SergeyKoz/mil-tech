#include <gtest/gtest.h>
#include <iostream>

#include "ballistics_simulator/scenario_loader.hpp"
#include "ballistics_simulator/world_explorer.hpp"
#include "ballistics_simulator/world_model.hpp"

namespace {

class WorldModel {
  public:
    WorldModel(ballistics_simulator::Scenario scenario)
        : scenario_(std::move(scenario)){};

    ballistics_simulator::LocalScanData getLocalScanData(ballistics_simulator::Position pos)
    {
        ballistics_simulator::LocalScanData scan;
        scan.scenario_name = scenario_.name;
        scan.robot = pos;

        for (int y = pos.y - scenario_.scan_radius; y <= pos.y + scenario_.scan_radius; ++y) {
            for (int x = pos.x - scenario_.scan_radius; x <= pos.x + scenario_.scan_radius; ++x) {
                const ballistics_simulator::Position position{x, y};

                if (!scenario_.in_bounds(position)) {
                    continue;
                }

                ballistics_simulator::ObservedCell observed;
                observed.position = position;
                observed.kind = scenario_.base_cell(position);
                scan.cells.push_back(observed);
            }
        };

        return scan;
    }

  private:
    ballistics_simulator::Scenario scenario_;
};

std::filesystem::path scenario_path(const std::string& filename)
{
    return std::filesystem::path(UNDERGROUND_WORLD_SOURCE_DIR) / "config" / filename;
}

ballistics_simulator::CellEnvironment create_cell_environment(const ballistics_simulator::LocalScanData& localScanData)
{
    std::array<ballistics_simulator::CellType, 9> cellTypes;

    std::transform(localScanData.cells.begin(),
                   localScanData.cells.begin() + 9,
                   cellTypes.begin(),
                   [](const ballistics_simulator::ObservedCell& item) {
                       return item.kind == ballistics_simulator::CellKind::Wall ? ballistics_simulator::CellType::Wall
                                                                                : ballistics_simulator::CellType::Free;
                   });

    auto pos = localScanData.cells.at(4).position;

    return {.position = {.x = pos.x, .y = pos.y}, .cellTypes = cellTypes};
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

TEST(WorldExplorerTest, MainLogic)
{
    auto scenario = ballistics_simulator::load_scenario(scenario_path("small_rooms.yaml"));

    WorldModel world(scenario);
    auto localScanData = world.getLocalScanData(scenario.start);

    auto _rootCell =
        ballistics_simulator::Cell{.position = {.x = localScanData.cells.at(4).position.x, .y = localScanData.cells.at(4).position.y},
                                   .type = ballistics_simulator::CellType::Free,
                                   .cells = {}};

    auto explorer = ballistics_simulator::WorldExplorer{_rootCell};
    explorer.applyEnvironment(create_cell_environment(localScanData));

    explorer.move(ballistics_simulator::MoveDirection::Down);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 1, .y = 2})));
    explorer.move(ballistics_simulator::MoveDirection::Down);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 1, .y = 3})));
    explorer.move(ballistics_simulator::MoveDirection::Down);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 1, .y = 4})));
    explorer.move(ballistics_simulator::MoveDirection::Down);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 1, .y = 5})));
    explorer.move(ballistics_simulator::MoveDirection::Right);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 2, .y = 5})));
    explorer.move(ballistics_simulator::MoveDirection::Right);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 3, .y = 5})));
    explorer.move(ballistics_simulator::MoveDirection::Right);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 4, .y = 5})));
    explorer.move(ballistics_simulator::MoveDirection::Right);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 5, .y = 5})));
    explorer.move(ballistics_simulator::MoveDirection::Right);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 6, .y = 5})));
    explorer.move(ballistics_simulator::MoveDirection::Right);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 7, .y = 5})));
    explorer.move(ballistics_simulator::MoveDirection::Up);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 7, .y = 4})));
    explorer.move(ballistics_simulator::MoveDirection::Up);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 7, .y = 3})));
    explorer.move(ballistics_simulator::MoveDirection::Right);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 8, .y = 3})));
    explorer.move(ballistics_simulator::MoveDirection::Right);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 9, .y = 3})));
    explorer.move(ballistics_simulator::MoveDirection::Up);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 9, .y = 2})));
    explorer.move(ballistics_simulator::MoveDirection::Up);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 9, .y = 1})));
    explorer.move(ballistics_simulator::MoveDirection::Left);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 8, .y = 1})));
    explorer.move(ballistics_simulator::MoveDirection::Left);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 7, .y = 1})));
    explorer.move(ballistics_simulator::MoveDirection::Left);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 6, .y = 1})));
    explorer.move(ballistics_simulator::MoveDirection::Left);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 5, .y = 1})));
    explorer.move(ballistics_simulator::MoveDirection::Down);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 5, .y = 2})));
    explorer.move(ballistics_simulator::MoveDirection::Down);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 5, .y = 3})));
    explorer.move(ballistics_simulator::MoveDirection::Right);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 6, .y = 3})));
    explorer.move(ballistics_simulator::MoveDirection::Left);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 5, .y = 3})));
    explorer.move(ballistics_simulator::MoveDirection::Left);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 4, .y = 3})));
    explorer.move(ballistics_simulator::MoveDirection::Left);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 3, .y = 3})));
    explorer.move(ballistics_simulator::MoveDirection::Up);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 3, .y = 2})));
    explorer.move(ballistics_simulator::MoveDirection::Up);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 3, .y = 1})));
    explorer.move(ballistics_simulator::MoveDirection::Left);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 2, .y = 1})));

    auto movements = explorer.goTo({.x = 1, .y = 1});

    EXPECT_EQ(1, movements.size());

    //   - "###########"
    //   - "#S..#.....#"
    //   - "#.#.#.###.#"
    //   - "#.#...C...#"
    //   - "#.#####.###"
    //   - "#...C.....#"
    //   - "###########"
}

TEST(WorldExplorerTest, Explore)
{
    auto scenario = ballistics_simulator::load_scenario(scenario_path("dead_end_bunker.yaml"));

    WorldModel world(scenario);
    auto localScanData = world.getLocalScanData(scenario.start);

    auto _rootCell =
        ballistics_simulator::Cell{.position = {.x = localScanData.cells.at(4).position.x, .y = localScanData.cells.at(4).position.y},
                                   .type = ballistics_simulator::CellType::Free,
                                   .cells = {}};

    auto explorer = ballistics_simulator::WorldExplorer{_rootCell};
    explorer.applyEnvironment(create_cell_environment(localScanData));

    bool mapExplored = false;

    while (!mapExplored) {
        auto direction = explorer.nextStep();

        if (direction == ballistics_simulator::MoveDirection::Stop) {
            mapExplored = true;

            break;
        }

        explorer.move(direction);

        auto currentPosition = explorer.getCurrent()->position;

        std::cout << moveLabel(direction) << " << x,y = " << currentPosition.x << "," << currentPosition.y << std::endl;

        explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = currentPosition.x, .y = currentPosition.y})));
    }

    auto movements = explorer.goToStart();

    while (!movements.empty()) {
        const auto& move = movements.front();
        explorer.move(move);
        movements.pop();
    }

    auto currentPosition = explorer.getCurrent()->position;

    //   - "###############"
    //   - "#S....#.......#"
    //   - "#.###.#.#####.#"
    //   - "#...#.#...C...#"
    //   - "###.#.###.###.#"
    //   - "#...#.....#...#"
    //   - "#.#######.#.###"
    //   - "#C......#.#...#"
    //   - "#######.#.###.#"
    //   - "#.......#...C.#"
    //   - "###############"

    EXPECT_TRUE(mapExplored);
    EXPECT_EQ(currentPosition, _rootCell.position);
}

}  // namespace
