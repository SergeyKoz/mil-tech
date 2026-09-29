#pragma once

#include "ballistics_simulator/common.hpp"

class ITargetsProvider
{
public:
  virtual auto isReady() -> bool = 0;
  virtual auto getTargetsCount() -> int = 0;
  virtual auto getTarget(int index) -> ballistics_simulator::TargetTelemetry = 0;
  virtual ~ITargetsProvider() = default;
};
