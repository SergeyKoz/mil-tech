#pragma once

#include "interfaces/drone_state_interface.hpp"
#include "ballistics_simulator/common.hpp"

// class TargetSelector;

namespace ballistics_simulator
{

  class TurningState : public IDroneState
  {
  public:
    // TurningState(TargetSelector& targetSelector);
    // [[deprecated]] auto execute(DroneContext& context) -> std::unique_ptr<IDroneState> override;
    auto execute(DroneContext &context) -> DroneCommand override;

    // private:
    //   TargetSelector* targetSelector;
  };

} // namespace ballistics_simulator