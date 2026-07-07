#pragma once

#include "interfaces/IDroneState.hpp"
#include "common.hpp"

class TargetSelector;

class TurningState : public IDroneState {
  public:
    TurningState(TargetSelector& targetSelector);
    auto execute(DroneContext& context) -> std::unique_ptr<IDroneState> override;

  private:
    TargetSelector* targetSelector;
};