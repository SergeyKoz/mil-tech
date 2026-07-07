#pragma once

#include "interfaces/ITargetsProvider.hpp"
#include <string>
#include <vector>

class JsonTargetProvider : public ITargetsProvider {
  public:
    JsonTargetProvider(std::string jsonFilePath);
    void load() override;
    int getTargetsCount() override;
    int getTimeSteps() override;
    Target getTarget(int index) override;

  private:
    std::string jsonFilePath;
    int targetsCount;
    int timeSteps;
    std::vector<Target> targets;
};