#pragma once

#include <fstream>
#include "common.hpp"
#include <httplib.h>

struct TestFlleInfo {
    bool found;
    std::string studentId;
    std::string testId;
    std::string uploadedAt;
    int steps;
};

class TestFilesStorage {
  public:
    TestFilesStorage(std::string studentId, TestsStorageConfig testsStorageConfig);
    auto send(TestCode testCode, std::ifstream simulationFile) -> void;

  private:
    static constexpr long retryDelay = 1;  // seconds
    std::string studentId;
    TestsStorageConfig testsStorageConfig;

    auto loadSimulationContent(TestCode testCode, std::ifstream& simulationFile) -> std::string;
    static auto parseFileInfo(const std::string& fileInfoContent) -> TestFlleInfo;
    auto createClient() -> httplib::Client;
};
