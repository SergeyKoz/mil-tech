#pragma once

#include "interfaces/ISimulationExport.hpp"
#include <fstream>

class JsonSimulationExport : public ISimulationExport {
  public:
    JsonSimulationExport(std::ofstream simulationFile);
    void dumpResults(int steps, std::array<SimStep, ThreadMissionProcessor::MAX_STEPS>& results) override;

  private:
    std::ofstream simulationFile;
};