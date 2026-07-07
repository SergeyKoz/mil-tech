#pragma once

#include "ITargetsProvider.hpp"
#include <string>

class JsonTargetProvider : public ITargetsProvider {
  public:
    JsonTargetProvider(const char* jsonFilePath);
    void load() override;
    virtual int getTargetsCount() override;
    virtual int getTimeSteps() override;
    virtual Target getTarget(int index) override;
    virtual ~JsonTargetProvider() override;

  private:
    std::string jsonFilePath;
    int targetsCount;
    int timeSteps;
    Target* targets;
};