#pragma once

#include "interfaces/ISimulationExport.hpp"
#include <string>

class JsonSimulationExport : public ISimulationExport {
  public:
    JsonSimulationExport(std::string jsonFilePath);
    void dumpResults(int steps, std::array<SimStep, ThreadMissionProcessor::MAX_STEPS>& results) override;

  private:
    std::string jsonFilePath;
};