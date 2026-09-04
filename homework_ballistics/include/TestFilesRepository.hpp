#pragma once

#include <string>
#include <fstream>
#include "common.hpp"

class TestFilesRepository {
  public:
    TestFilesRepository(std::string fileRepositoryPath);
    auto setTest(TestCode test) -> void;
    auto getConfigFile() -> std::ifstream;
    auto getTargetsFile() -> std::ifstream;
    auto putSimulationFile() -> std::ofstream;
    auto getSimulationFile() -> std::ifstream;
    auto hasSimulationFile() -> bool;

  private:
    std::string fileRepositoryPath;
    TestCode test{TestCode::T1};
};
