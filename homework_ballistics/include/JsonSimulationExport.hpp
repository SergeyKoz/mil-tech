#pragma once

#include "ISimulationExport.hpp"
#include <string>

class JsonSimulationExport : public ISimulationExport {
  public:
    JsonSimulationExport(const char* jsonFilePath);
    void dumpResults(int steps, const SimStep* results) override;

  private:
    std::string jsonFilePath;
};