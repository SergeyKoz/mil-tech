#pragma once

#include "interfaces/IDroneState.hpp"

class TargetSelector;

class StoppedState : public IDroneState {
  public:
    StoppedState(TargetSelector& targetSelector);
    auto execute(DroneContext& context) -> std::unique_ptr<IDroneState> override;

  private:
    TargetSelector* targetSelector;
};