#pragma once

#include "interfaces/IDroneState.hpp"

class TargetSelector;

class AcceleratingState : public IDroneState {
  public:
    AcceleratingState(TargetSelector& targetSelector);
    [[deprecated]] auto execute(DroneContext& context) -> std::unique_ptr<IDroneState> override;
    auto threadExecute(DroneContext& context) -> DroneCommand override;

  private:
    TargetSelector* targetSelector;
};