#pragma once

#include "common.hpp"

class ITargetsProvider {
  public:
    virtual auto load() -> void = 0;
    virtual auto getTargetsCount() -> int = 0;
    virtual auto getTimeSteps() -> int = 0;
    virtual auto getTarget(int index) -> Target* = 0;
    virtual ~ITargetsProvider() = default;
};