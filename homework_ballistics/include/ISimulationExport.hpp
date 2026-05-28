#pragma once

struct SimStep;

class ISimulationExport {
  public:
    virtual void dumpResults(int steps, const SimStep* results) = 0;
    virtual ~ISimulationExport() = default;
};