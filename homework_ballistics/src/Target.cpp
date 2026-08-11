#include "Target.hpp"

Target::Target(std::vector<Coord> positions, int timeSteps)
    : positions(positions)
    , timeSteps(timeSteps)
    , telemetry({positions.at(0), {0.F, 0.F}}){};

auto Target::getTelemetry() const -> TargetTelemetry
{
    std::lock_guard<std::mutex> lock(dataMutex);

    return telemetry;
};

auto Target::setTelemetry(const TargetTelemetry& telemetry) -> void
{
    std::lock_guard<std::mutex> lock(dataMutex);

    this->telemetry = telemetry;
};

auto Target::update(float time, float timeStep, float simulationStep) -> void
{
    int index = static_cast<int>(std::floor(time / timeStep));
    int _current = index % timeSteps;
    int _next = (_current + 1) % timeSteps;
    float frac = (time - static_cast<float>(index) * timeStep) / timeStep;
    Coord current = positions[_current];
    Coord next = positions[_next];
    Coord nextPosition = {.x = current.x + (next.x - current.x) * frac, .y = current.y + (next.y - current.y) * frac};

    std::lock_guard<std::mutex> lock(dataMutex);

    telemetry = {.position = nextPosition,
                 .speed = {.x = (nextPosition.x - telemetry.position.x) / simulationStep,
                           .y = (nextPosition.y - telemetry.position.y) / simulationStep}};
};
