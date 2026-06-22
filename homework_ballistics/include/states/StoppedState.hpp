#pragma once

#include "interfaces/IDroneState.hpp"

class TargetSelector;

class StoppedState : public IDroneState {
  public:
    StoppedState(TargetSelector& targetSelector);
    [[deprecated]] auto execute(DroneContext& context) -> std::unique_ptr<IDroneState> override;
    auto threadExecute(DroneContext& context) -> DroneCommand override;

  private:
    TargetSelector* targetSelector;
};