#pragma once

#include "common.hpp"

class ITargetProcessor {
  public:
    virtual auto processTarget(const SelectedTarget& target, const DroneTelemetry& droneTelemetry) -> void = 0;
    virtual ~ITargetProcessor() = default;
};