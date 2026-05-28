#pragma once

#include "MissionProcessor.hpp"

struct SimStep;

class ISimulationExport {
  public:
    virtual void dumpResults(int steps, std::array<SimStep, MissionProcessor::MAX_STEPS>& results) = 0;
    virtual ~ISimulationExport() = default;
};