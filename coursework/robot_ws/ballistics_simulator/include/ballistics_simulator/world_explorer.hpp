#pragma once

#include <array>
#include <queue>
#include <unordered_map>

namespace ballistics_simulator {

struct CellPosition {
    int x;
    int y;

    bool operator==(const CellPosition& other) const { return x == other.x && y == other.y; }
    bool operator!=(const CellPosition& other) const { return x != other.x || y != other.y; }
};

struct CellPositionHash {
    std::size_t operator()(const CellPosition& pos) const
    {
        std::size_t h1 = std::hash<int>{}(pos.x);
        std::size_t h2 = std::hash<int>{}(pos.y);
        return h1 ^ (h2 << 1);
    }
};

enum class CellType {
    Wall,
    Free,
    // Start,
    // Contact,
    // ProcessedContact,
};

enum class MoveDirection { Up, Down, Left, Right, Stop };

struct Cell {
    CellPosition position;
    CellType type;
    std::array<Cell*, 8> cells;
};

struct CellEnvironment {
    CellPosition position;
    std::array<CellType, 9> cellTypes;
};

// a model which allows to work with a map:
// - build map
// - move in the map
// - explore map
// - find a shortest path in the map
class WorldExplorer {
  public:
    explicit WorldExplorer();
    explicit WorldExplorer(Cell startCell);

    // set a root node
    void init(Cell startCell);

    // current position in the graph
    Cell* getCurrent() const;

    // apply surrounded cells to a graph: Free, Wall
    void applyEnvironment(CellEnvironment cellEnvironment);

    // move to a neighbor cell: Up, Down, Left, Right, Stop
    void move(MoveDirection direction);

    // build moves sequence to a specyfic already explored cell
    std::queue<MoveDirection> goTo(CellPosition targetPosition);

    // graph traversal step, returns direction for next step, if graph explored, returns: Stop
    //
    // ballistics_simulator::WorldExplorer explorer{rootCell};
    //
    // bool mapExplored = false;
    //
    // while (!mapExplored) {
    //     auto direction = explorer.nextStep();
    //     if (direction == ballistics_simulator::MoveDirection::Stop) {
    //         mapExplored = true;
    //
    //         break;
    //     }
    //
    //     explorer.move(direction);
    //     auto currentPosition = explorer.getCurrent()->position;
    //     explorer.applyEnvironment(... build CellEnvironment for currentPosition ...);
    // }
    MoveDirection nextStep();

    // Builds moves sequence from a current position to root cell
    //
    // WorldExplorer explorer;
    // explorer.init(rootCell);
    //
    // ... exploring process ...
    //
    // auto movements = explorer.goToStart();
    //
    // while (!movements.empty()) {
    //     const auto& move = movements.front();
    //     explorer.move(move);
    //     movements.pop();
    // }
    std::queue<MoveDirection> goToStart();

  private:
    Cell root;
    Cell* current;
    std::unordered_map<CellPosition, Cell*, CellPositionHash> explored;
    std::unordered_map<CellPosition, Cell*, CellPositionHash> unexplored;

    MoveDirection getDirection(const CellPosition& from, const CellPosition& to);
};

}  // namespace ballistics_simulator
