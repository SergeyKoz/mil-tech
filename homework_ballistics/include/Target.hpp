#pragma once

#include "common.hpp"
#include <thread>
#include <vector>

class Target {
  public:
    Target(std::vector<Coord> positions, int timeSteps);

    auto update(float time, float timeStep, float simulationStep) -> void;

    Coord position;
    Speed velocity;

  private:
    std::mutex dataMutex;
    std::vector<Coord> positions;
    int timeSteps;
};
