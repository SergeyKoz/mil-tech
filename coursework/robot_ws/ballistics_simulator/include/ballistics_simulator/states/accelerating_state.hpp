#pragma once

// #include "interfaces/IDroneState.hpp"
#include "interfaces/drone_state_interface.hpp"

// class TargetSelector;

namespace ballistics_simulator
{

  class AcceleratingState : public IDroneState
  {
  public:
    // AcceleratingState(TargetSelector& targetSelector);
    // [[deprecated]] auto execute(DroneContext& context) -> std::unique_ptr<IDroneState> override;
    auto execute(DroneContext &context) -> DroneCommand override;

    // private:
    //   TargetSelector* targetSelector;
  };

} // namespace ballistics_simulator