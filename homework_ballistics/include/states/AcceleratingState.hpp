#pragma once

#include "interfaces/IDroneState.hpp"

class TargetSelector;

class AcceleratingState : public IDroneState {
  public:
    AcceleratingState(TargetSelector& targetSelector);
    auto execute(DroneContext& context) -> std::unique_ptr<IDroneState> override;

  private:
    TargetSelector* targetSelector;
};