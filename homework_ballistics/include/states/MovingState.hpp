#pragma once

#include "interfaces/IDroneState.hpp"

class TargetSelector;

class MovingState : public IDroneState {
  public:
    MovingState(TargetSelector& targetSelector);
    auto execute(DroneContext& context) -> std::unique_ptr<IDroneState> override;

  private:
    TargetSelector* targetSelector;
};