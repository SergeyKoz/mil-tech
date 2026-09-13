#pragma once

#include "interfaces/drone_state_interface.hpp"

namespace ballistics_simulator
{

  class AcceleratingState : public IDroneState
  {
  public:
    auto execute(DroneContext &context) -> DroneCommand override;
  };

} // namespace ballistics_simulator