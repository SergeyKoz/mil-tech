#pragma once

#include "interfaces/ITargetsProvider.hpp"
#include "Target.hpp"
#include <memory>
#include <vector>
#include <unordered_map>

class CheckerTargetProvider : public ITargetsProvider {
  public:
    auto load() -> void override;
    auto getTargetsCount() -> int override;
    auto getTimeSteps() -> int override;
    auto getTarget(int index) -> Target* override;

    auto isReady() -> bool;
    auto setTarget(int index, Coord pos, float time) -> void;
    auto setTargetsCount(int count) -> void;

  private:
    int targetsCount;
    std::unordered_map<int, float> currentTargetsTimes;
    std::unordered_map<int, float> previousTargetsTimes;
    std::unordered_map<int, std::unique_ptr<Target>> currentTargets;
    std::unordered_map<int, std::unique_ptr<Target>> previousTargets;
};