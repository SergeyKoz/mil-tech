#include <array>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>
#include <algorithm>
#include "underground_world/world_explorer.hpp"

namespace underground_world {

WorldExplorer::WorldExplorer()
    : root({})
    , current(nullptr){};

WorldExplorer::WorldExplorer(Cell startCell)
    : root(startCell)
    , current(&root)
{
    explored[startCell.position] = &root;
};

void WorldExplorer::init(Cell startCell)
{
    root = startCell;
    current = &root;
    explored[startCell.position] = &root;
}

Cell* WorldExplorer::getCurrent() const
{
    return current;
}

void WorldExplorer::applyEnvironment(CellEnvironment cellEnvironment)
{
    auto position = current->position;

    // 1 row
    if (current->cells.at(0) == nullptr) {
        auto pos = CellPosition{.x = position.x - 1, .y = position.y - 1};

        if (explored.contains(pos)) {
            current->cells.at(0) = explored.at(pos);
        }
        else {
            current->cells.at(0) = new underground_world::Cell{.position = pos, .type = cellEnvironment.cellTypes.at(0), .cells{}};

            if (current->cells.at(0)->type != CellType::Wall) {
                unexplored[pos] = current->cells.at(0);
            }
        }
    }

    if (current->cells.at(1) == nullptr) {
        auto pos = CellPosition{.x = position.x, .y = position.y - 1};

        if (explored.contains(pos)) {
            current->cells.at(1) = explored.at(pos);
        }
        else {
            current->cells.at(1) = new underground_world::Cell{.position = pos, .type = cellEnvironment.cellTypes.at(1), .cells{}};

            if (current->cells.at(1)->type != CellType::Wall) {
                unexplored[pos] = current->cells.at(1);
            }
        }
    }

    if (current->cells.at(2) == nullptr) {
        auto pos = CellPosition{.x = position.x + 1, .y = position.y - 1};

        if (explored.contains(pos)) {
            current->cells.at(2) = explored.at(pos);
        }
        else {
            current->cells.at(2) = new underground_world::Cell{.position = pos, .type = cellEnvironment.cellTypes.at(2), .cells{}};

            if (current->cells.at(2)->type != CellType::Wall) {
                unexplored[pos] = current->cells.at(2);
            }
        }
    }

    // 2 row

    if (current->cells.at(3) == nullptr) {
        auto pos = CellPosition{.x = position.x - 1, .y = position.y};

        if (explored.contains(pos)) {
            current->cells.at(3) = explored.at(pos);
        }
        else {
            current->cells.at(3) = new underground_world::Cell{.position = pos, .type = cellEnvironment.cellTypes.at(3), .cells{}};

            if (current->cells.at(3)->type != CellType::Wall) {
                unexplored[pos] = current->cells.at(3);
            }
        }
    }

    if (current->cells.at(4) == nullptr) {
        auto pos = CellPosition{.x = position.x + 1, .y = position.y};

        if (explored.contains(pos)) {
            current->cells.at(4) = explored.at(pos);
        }
        else {
            current->cells.at(4) = new underground_world::Cell{.position = pos, .type = cellEnvironment.cellTypes.at(5), .cells{}};

            if (current->cells.at(4)->type != CellType::Wall) {
                unexplored[pos] = current->cells.at(4);
            }
        }
    }

    // 3 row
    if (current->cells.at(5) == nullptr) {
        auto pos = CellPosition{.x = position.x - 1, .y = position.y + 1};

        if (explored.contains(pos)) {
            current->cells.at(5) = explored.at(pos);
        }
        else {
            current->cells.at(5) = new underground_world::Cell{.position = pos, .type = cellEnvironment.cellTypes.at(6), .cells{}};

            if (current->cells.at(5)->type != CellType::Wall) {
                unexplored[pos] = current->cells.at(5);
            }
        }
    }

    if (current->cells.at(6) == nullptr) {
        auto pos = CellPosition{.x = position.x, .y = position.y + 1};

        if (explored.contains(pos)) {
            current->cells.at(6) = explored.at(pos);
        }
        else {
            current->cells.at(6) = new underground_world::Cell{.position = pos, .type = cellEnvironment.cellTypes.at(7), .cells{}};

            if (current->cells.at(6)->type != CellType::Wall) {
                unexplored[pos] = current->cells.at(6);
            }
        }
    }

    if (current->cells.at(7) == nullptr) {
        auto pos = CellPosition{.x = position.x + 1, .y = position.y + 1};

        if (explored.contains(pos)) {
            current->cells.at(7) = explored.at(pos);
        }
        else {
            current->cells.at(7) = new underground_world::Cell{.position = pos, .type = cellEnvironment.cellTypes.at(8), .cells{}};

            if (current->cells.at(7)->type != CellType::Wall) {
                unexplored[pos] = current->cells.at(7);
            }
        }
    }

    // 0 1 2
    // 3   4
    // 5 6 7

    // cell0.cells.at(4) = &cell1;
    // cell0.cells.at(6) = &rootCell;
    // cell0.cells.at(7) = &cell3;

    auto cells0 = current->cells.at(0)->cells;

    if (cells0.at(4) == nullptr) {
        current->cells.at(0)->cells.at(4) = current->cells.at(1);
    }

    if (cells0.at(6) == nullptr) {
        current->cells.at(0)->cells.at(6) = current->cells.at(3);
    }

    if (cells0.at(7) == nullptr) {
        current->cells.at(0)->cells.at(7) = current;
    }

    // cell1.cells.at(3) = &cell0;
    // cell1.cells.at(4) = &cell2;
    // cell1.cells.at(5) = &cell3;
    // cell1.cells.at(6) = &rootCell;
    // cell1.cells.at(7) = &cell4;

    auto cells1 = current->cells.at(1)->cells;

    if (cells1.at(3) == nullptr) {
        current->cells.at(1)->cells.at(3) = current->cells.at(0);
    }

    if (cells1.at(4) == nullptr) {
        current->cells.at(1)->cells.at(4) = current->cells.at(2);
    }

    if (cells1.at(5) == nullptr) {
        current->cells.at(1)->cells.at(5) = current->cells.at(3);
    }

    if (cells1.at(6) == nullptr) {
        current->cells.at(1)->cells.at(6) = current;
    }

    if (cells1.at(7) == nullptr) {
        current->cells.at(1)->cells.at(7) = current->cells.at(4);
    }

    // cell2.cells.at(3) = &cell1;
    // cell2.cells.at(5) = &rootCell;
    // cell2.cells.at(6) = &cell4;

    auto cells2 = current->cells.at(2)->cells;

    if (cells2.at(3) == nullptr) {
        current->cells.at(2)->cells.at(3) = current->cells.at(1);
    }

    if (cells2.at(5) == nullptr) {
        current->cells.at(2)->cells.at(5) = current;
    }

    if (cells2.at(6) == nullptr) {
        current->cells.at(2)->cells.at(6) = current->cells.at(4);
    }

    // cell3.cells.at(1) = &cell0;
    // cell3.cells.at(2) = &cell1;
    // cell3.cells.at(4) = &rootCell;
    // cell3.cells.at(6) = &cell5;
    // cell3.cells.at(7) = &cell6;

    auto cells3 = current->cells.at(3)->cells;

    if (cells3.at(1) == nullptr) {
        current->cells.at(3)->cells.at(1) = current->cells.at(0);
    }

    if (cells3.at(2) == nullptr) {
        current->cells.at(3)->cells.at(2) = current->cells.at(1);
    }

    if (cells3.at(4) == nullptr) {
        current->cells.at(3)->cells.at(4) = current;
    }

    if (cells3.at(6) == nullptr) {
        current->cells.at(3)->cells.at(6) = current->cells.at(5);
    }

    if (cells3.at(7) == nullptr) {
        current->cells.at(3)->cells.at(7) = current->cells.at(6);
    }

    // cell4.cells.at(0) = &cell1;
    // cell4.cells.at(1) = &cell2;
    // cell4.cells.at(3) = &rootCell;
    // cell4.cells.at(5) = &cell6;
    // cell4.cells.at(6) = &cell7;

    auto cells4 = current->cells.at(4)->cells;

    if (cells4.at(0) == nullptr) {
        current->cells.at(4)->cells.at(0) = current->cells.at(1);
    }

    if (cells4.at(1) == nullptr) {
        current->cells.at(4)->cells.at(1) = current->cells.at(2);
    }

    if (cells4.at(3) == nullptr) {
        current->cells.at(4)->cells.at(3) = current;
    }

    if (cells4.at(5) == nullptr) {
        current->cells.at(4)->cells.at(5) = current->cells.at(6);
    }

    if (cells4.at(6) == nullptr) {
        current->cells.at(4)->cells.at(6) = current->cells.at(7);
    }

    // cell5.cells.at(1) = &cell3;
    // cell5.cells.at(2) = &rootCell;
    // cell5.cells.at(4) = &cell6;

    auto cells5 = current->cells.at(5)->cells;

    if (cells5.at(1) == nullptr) {
        current->cells.at(5)->cells.at(1) = current->cells.at(3);
    }

    if (cells5.at(2) == nullptr) {
        current->cells.at(5)->cells.at(2) = current;
    }

    if (cells5.at(4) == nullptr) {
        current->cells.at(5)->cells.at(4) = current->cells.at(6);
    }

    // cell6.cells.at(0) = &cell3;
    // cell6.cells.at(1) = &rootCell;
    // cell6.cells.at(2) = &cell4;
    // cell6.cells.at(3) = &cell5;
    // cell6.cells.at(4) = &cell7;

    auto cells6 = current->cells.at(6)->cells;

    if (cells6.at(0) == nullptr) {
        current->cells.at(6)->cells.at(0) = current->cells.at(3);
    }

    if (cells6.at(1) == nullptr) {
        current->cells.at(6)->cells.at(1) = current;
    }

    if (cells6.at(2) == nullptr) {
        current->cells.at(6)->cells.at(2) = current->cells.at(4);
    }

    if (cells6.at(3) == nullptr) {
        current->cells.at(6)->cells.at(3) = current->cells.at(5);
    }

    if (cells6.at(4) == nullptr) {
        current->cells.at(6)->cells.at(4) = current->cells.at(7);
    }

    // cell7.cells.at(0) = &rootCell;
    // cell7.cells.at(1) = &cell4;
    // cell7.cells.at(3) = &cell6;

    auto cells7 = current->cells.at(7)->cells;

    if (cells7.at(0) == nullptr) {
        current->cells.at(7)->cells.at(0) = current;
    }

    if (cells7.at(1) == nullptr) {
        current->cells.at(7)->cells.at(1) = current->cells.at(4);
    }

    if (cells7.at(2) == nullptr) {
        current->cells.at(7)->cells.at(3) = current->cells.at(6);
    }
};

void WorldExplorer::move(MoveDirection direction)
{
    switch (direction) {
        case MoveDirection::Up:
            if (current->cells.at(1) != nullptr && current->cells.at(1)->type == CellType::Free) {
                current = current->cells.at(1);

                explored[current->position] = current;
                unexplored.erase(current->position);
            }
            else {
                throw std::runtime_error("Wrong move");
            }

            break;
        case MoveDirection::Down:
            if (current->cells.at(6) != nullptr && current->cells.at(6)->type == CellType::Free) {
                current = current->cells.at(6);

                explored[current->position] = current;
                unexplored.erase(current->position);
            }
            else {
                throw std::runtime_error("Wrong move");
            }

            break;
        case MoveDirection::Left:
            if (current->cells.at(3) != nullptr && current->cells.at(3)->type == CellType::Free) {
                current = current->cells.at(3);

                explored[current->position] = current;
                unexplored.erase(current->position);
            }
            else {
                throw std::runtime_error("Wrong move");
            }

            break;
        case MoveDirection::Right:
            if (current->cells.at(4) != nullptr && current->cells.at(4)->type == CellType::Free) {
                current = current->cells.at(4);

                explored[current->position] = current;
                unexplored.erase(current->position);
            }
            else {
                throw std::runtime_error("Wrong move");
            }

            break;
        case MoveDirection::Stop:
        default:
            break;
    }
}

std::queue<MoveDirection> WorldExplorer::goTo(CellPosition targetPosition)
{
    if (current == nullptr || current->position == targetPosition) {
        return {};
    }

    std::queue<Cell*> queue;
    std::unordered_set<Cell*> visited;
    std::unordered_map<Cell*, Cell*> parent;  // child -> parent

    queue.push(current);
    visited.insert(current);

    Cell* targetCell = nullptr;

    // 1. Run BFS to find the target position
    while (!queue.empty()) {
        Cell* curr = queue.front();
        queue.pop();

        if (curr->position == targetPosition) {
            targetCell = curr;
            break;
        }

        for (Cell* neighbor : {curr->cells[1], curr->cells[3], curr->cells[4], curr->cells[6]}) {
            if (neighbor == nullptr || neighbor->type == CellType::Wall) {
                continue;  // Skip unexplored/null and walls
            }

            if (visited.find(neighbor) == visited.end()) {
                visited.insert(neighbor);
                parent[neighbor] = curr;
                queue.push(neighbor);
            }
        }
    }

    std::queue<MoveDirection> directions{};

    if (targetCell) {
        std::vector<Cell*> cellPath;
        Cell* step = targetCell;

        while (step != current) {
            cellPath.push_back(step);
            step = parent[step];
        }

        cellPath.push_back(current);

        std::reverse(cellPath.begin(), cellPath.end());

        for (size_t i = 0; i < cellPath.size() - 1; ++i) {
            directions.push(getDirection(cellPath[i]->position, cellPath[i + 1]->position));
        }
    }

    return directions;
}

MoveDirection WorldExplorer::nextStep()
{
    if (unexplored.empty()) {
        return MoveDirection::Stop;
    }

    std::queue<MoveDirection> movements = goTo(unexplored.begin()->first);

    if (movements.size() == 0) {
        throw std::runtime_error("Can't get position");
    }

    return movements.front();
}

std::queue<MoveDirection> WorldExplorer::goToStart()
{
    return goTo(root.position);
}

MoveDirection WorldExplorer::getDirection(const CellPosition& from, const CellPosition& to)
{
    if (to.y < from.y) {
        return MoveDirection::Up;
    }

    if (to.y > from.y) {
        return MoveDirection::Down;
    }

    if (to.x < from.x) {
        return MoveDirection::Left;
    }

    return MoveDirection::Right;  // to.x > from.x
}

}  // namespace underground_world
