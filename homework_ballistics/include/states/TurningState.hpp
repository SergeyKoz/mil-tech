#pragma once

#include "interfaces/IDroneState.hpp"
#include "common.hpp"

class TargetSelector;

class TurningState : public IDroneState {
  public:
    TurningState(TargetSelector& targetSelector);
    [[deprecated]] auto execute(DroneContext& context) -> std::unique_ptr<IDroneState> override;
    auto threadExecute(DroneContext& context) -> DroneCommand override;

  private:
    TargetSelector* targetSelector;
};