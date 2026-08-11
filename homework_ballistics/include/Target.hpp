#pragma once

#include "common.hpp"
#include <thread>
#include <vector>

class Target {
  public:
    Target(std::vector<Coord> positions, int timeSteps);

    auto update(float time, float timeStep, float simulationStep) -> void;
    auto getTelemetry() const -> TargetTelemetry;
    auto setTelemetry(const TargetTelemetry& telemetry) -> void;

  private:
    mutable std::mutex dataMutex;
    std::vector<Coord> positions;
    int timeSteps;
    TargetTelemetry telemetry;
};
