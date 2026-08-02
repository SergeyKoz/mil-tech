#pragma once

#include "ThreadMissionProcessor.hpp"

struct SimStep;

class ISimulationExport {
  public:
    virtual void dumpResults(int steps, std::array<SimStep, ThreadMissionProcessor::MAX_STEPS>& results) = 0;
    virtual ~ISimulationExport() = default;
};