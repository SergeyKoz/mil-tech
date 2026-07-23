#include <gtest/gtest.h>
#include <iostream>

#include "underground_world/scenario_loader.hpp"
#include "underground_world/world_explorer.hpp"
#include "underground_world/world_model.hpp"

namespace {

class WorldModel {
  public:
    WorldModel(underground_world::Scenario scenario)
        : scenario_(std::move(scenario)){};

    underground_world::LocalScanData getLocalScanData(underground_world::Position pos)
    {
        underground_world::LocalScanData scan;
        scan.scenario_name = scenario_.name;
        scan.robot = pos;

        for (int y = pos.y - scenario_.scan_radius; y <= pos.y + scenario_.scan_radius; ++y) {
            for (int x = pos.x - scenario_.scan_radius; x <= pos.x + scenario_.scan_radius; ++x) {
                const underground_world::Position position{x, y};

                if (!scenario_.in_bounds(position)) {
                    continue;
                }

                underground_world::ObservedCell observed;
                observed.position = position;
                observed.kind = scenario_.base_cell(position);
                scan.cells.push_back(observed);
            }
        };

        return scan;
    }

  private:
    underground_world::Scenario scenario_;
};

std::filesystem::path scenario_path(const std::string& filename)
{
    return std::filesystem::path(UNDERGROUND_WORLD_SOURCE_DIR) / "config" / filename;
}

underground_world::CellEnvironment create_cell_environment(const underground_world::LocalScanData& localScanData)
{
    std::array<underground_world::CellType, 9> cellTypes;

    std::transform(
        localScanData.cells.begin(), localScanData.cells.begin() + 9, cellTypes.begin(), [](const underground_world::ObservedCell& item) {
            return item.kind == underground_world::CellKind::Wall ? underground_world::CellType::Wall : underground_world::CellType::Free;
        });

    auto pos = localScanData.cells.at(4).position;

    return {.position = {.x = pos.x, .y = pos.y}, .cellTypes = cellTypes};
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

TEST(WorldExplorerTest, MainLogic)
{
    auto scenario = underground_world::load_scenario(scenario_path("small_rooms.yaml"));

    WorldModel world(scenario);
    auto localScanData = world.getLocalScanData(scenario.start);

    auto _rootCell =
        underground_world::Cell{.position = {.x = localScanData.cells.at(4).position.x, .y = localScanData.cells.at(4).position.y},
                                .type = underground_world::CellType::Free,
                                .cells = {}};

    auto explorer = underground_world::WorldExplorer{_rootCell};
    explorer.applyEnvironment(create_cell_environment(localScanData));

    explorer.move(underground_world::MoveDirection::Down);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 1, .y = 2})));
    explorer.move(underground_world::MoveDirection::Down);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 1, .y = 3})));
    explorer.move(underground_world::MoveDirection::Down);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 1, .y = 4})));
    explorer.move(underground_world::MoveDirection::Down);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 1, .y = 5})));
    explorer.move(underground_world::MoveDirection::Right);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 2, .y = 5})));
    explorer.move(underground_world::MoveDirection::Right);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 3, .y = 5})));
    explorer.move(underground_world::MoveDirection::Right);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 4, .y = 5})));
    explorer.move(underground_world::MoveDirection::Right);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 5, .y = 5})));
    explorer.move(underground_world::MoveDirection::Right);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 6, .y = 5})));
    explorer.move(underground_world::MoveDirection::Right);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 7, .y = 5})));
    explorer.move(underground_world::MoveDirection::Up);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 7, .y = 4})));
    explorer.move(underground_world::MoveDirection::Up);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 7, .y = 3})));
    explorer.move(underground_world::MoveDirection::Right);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 8, .y = 3})));
    explorer.move(underground_world::MoveDirection::Right);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 9, .y = 3})));
    explorer.move(underground_world::MoveDirection::Up);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 9, .y = 2})));
    explorer.move(underground_world::MoveDirection::Up);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 9, .y = 1})));
    explorer.move(underground_world::MoveDirection::Left);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 8, .y = 1})));
    explorer.move(underground_world::MoveDirection::Left);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 7, .y = 1})));
    explorer.move(underground_world::MoveDirection::Left);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 6, .y = 1})));
    explorer.move(underground_world::MoveDirection::Left);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 5, .y = 1})));
    explorer.move(underground_world::MoveDirection::Down);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 5, .y = 2})));
    explorer.move(underground_world::MoveDirection::Down);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 5, .y = 3})));
    explorer.move(underground_world::MoveDirection::Right);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 6, .y = 3})));
    explorer.move(underground_world::MoveDirection::Left);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 5, .y = 3})));
    explorer.move(underground_world::MoveDirection::Left);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 4, .y = 3})));
    explorer.move(underground_world::MoveDirection::Left);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 3, .y = 3})));
    explorer.move(underground_world::MoveDirection::Up);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 3, .y = 2})));
    explorer.move(underground_world::MoveDirection::Up);
    explorer.applyEnvironment(create_cell_environment(world.getLocalScanData({.x = 3, .y = 1})));
    explorer.move(underground_world::MoveDirection::Left);
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
    auto scenario = underground_world::load_scenario(scenario_path("dead_end_bunker.yaml"));

    WorldModel world(scenario);
    auto localScanData = world.getLocalScanData(scenario.start);

    auto _rootCell =
        underground_world::Cell{.position = {.x = localScanData.cells.at(4).position.x, .y = localScanData.cells.at(4).position.y},
                                .type = underground_world::CellType::Free,
                                .cells = {}};

    auto explorer = underground_world::WorldExplorer{_rootCell};
    explorer.applyEnvironment(create_cell_environment(localScanData));

    bool mapExplored = false;

    while (!mapExplored) {
        auto direction = explorer.nextStep();

        if (direction == underground_world::MoveDirection::Stop) {
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
