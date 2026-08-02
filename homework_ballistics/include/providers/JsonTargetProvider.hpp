#pragma once

#include "interfaces/ITargetsProvider.hpp"
#include "Target.hpp"
#include <memory>
#include <string>
#include <vector>

class JsonTargetProvider : public ITargetsProvider {
  public:
    JsonTargetProvider(std::string jsonFilePath);
    auto load() -> void override;
    auto getTargetsCount() -> int override;
    auto getTimeSteps() -> int override;
    auto getTarget(int index) -> Target* override;

  private:
    std::string jsonFilePath;
    int targetsCount;
    int timeSteps;
    std::vector<std::unique_ptr<Target>> targets;
};