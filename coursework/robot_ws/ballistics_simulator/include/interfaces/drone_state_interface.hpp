#pragma once

// struct DroneContext;
// struct DroneCommand;

#include "ballistics_simulator/common.hpp"

class IDroneState
{
public:
  virtual auto execute(ballistics_simulator::DroneContext &context) -> ballistics_simulator::DroneCommand = 0;
  virtual ~IDroneState() = default;
};