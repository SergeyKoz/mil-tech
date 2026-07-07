#pragma once

#include "common.hpp"

class ITargetsProvider {
  public:
    virtual void load() = 0;
    virtual int getTargetsCount() = 0;
    virtual int getTimeSteps() = 0;
    virtual Target getTarget(int index) = 0;
    virtual ~ITargetsProvider() = default;
};