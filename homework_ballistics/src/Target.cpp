#include "Target.hpp"

Target::Target(std::vector<Coord> positions)
    : position(positions.at(0))
    , velocity({0.F, 0.F})
    , positions(positions){};

auto Target::update(float time, float timeStep, float simulationStep) -> void
{
    int index = static_cast<int>(std::floor(time / timeStep));
    int _current = index % 60;
    int _next = (_current + 1) % 60;
    float frac = (time - static_cast<float>(index) * timeStep) / timeStep;
    Coord current = positions[_current];
    Coord next = positions[_next];

    Coord nextPosition = {.x = current.x + (next.x - current.x) * frac, .y = current.y + (next.y - current.y) * frac};

    std::lock_guard<std::mutex> lock(dataMutex);

    velocity = {.x = (nextPosition.x - position.x) / simulationStep, .y = (nextPosition.y - position.y) / simulationStep};
    position = nextPosition;
};
