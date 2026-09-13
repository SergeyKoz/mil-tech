#pragma once

#include "ballistics_simulator/common.hpp"

// struct TargetTelemetry;

class ITargetsProvider
{
public:
  virtual auto isReady() -> bool = 0;
  // virtual auto load() -> void = 0;
  virtual auto getTargetsCount() -> int = 0;
  // virtual auto setTarget(int index, ballistics_simulator::Coord pos) -> void;
  // virtual auto getTimeSteps() -> int = 0;
  virtual auto getTarget(int index) -> ballistics_simulator::TargetTelemetry = 0;
  virtual ~ITargetsProvider() = default;
};
