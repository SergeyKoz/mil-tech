#pragma once

#include "interfaces/IDroneState.hpp"

class TargetSelector;

class DeceleratingState : public IDroneState {
  public:
    DeceleratingState(TargetSelector& targetSelector);
    auto execute(DroneContext& context) -> std::unique_ptr<IDroneState> override;

  private:
    TargetSelector* targetSelector;
};