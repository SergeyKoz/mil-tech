#pragma once
#include "ballistics_simulator/common.hpp"

class IBallisticsSolver
{
public:
  virtual void init() = 0;
  virtual ballistics_simulator::DropParameters calcDropParameters(const ballistics_simulator::AmmoParams &ammo, float v0, float z0) = 0;
  virtual ~IBallisticsSolver() = default;
};