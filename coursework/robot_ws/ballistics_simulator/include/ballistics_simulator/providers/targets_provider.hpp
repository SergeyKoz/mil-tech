#pragma once

#include "interfaces/targets_provider_interface.hpp"
#include "ballistics_simulator/common.hpp"
#include "interfaces/loggable_interface.hpp"
#include <memory>
#include <vector>
#include <unordered_map>

namespace ballistics_simulator
{
  class TargetsProvider : public ITargetsProvider, public ILoggable
  {
  public:
    auto getTargetsCount() -> int override;
    auto getTarget(int index) -> TargetTelemetry override;
    auto isReady() -> bool override;
    auto setTarget(int index, Coord pos, float time) -> void;
    auto setTargetsCount(int count) -> void;

  private:
    int targetsCount{0};
    std::unordered_map<int, float> currentTargetsTimes;
    std::unordered_map<int, float> previousTargetsTimes;
    std::unordered_map<int, TargetTelemetry> currentTargets;
    std::unordered_map<int, TargetTelemetry> previousTargets;
  };
} // namespace ballistics_simulator