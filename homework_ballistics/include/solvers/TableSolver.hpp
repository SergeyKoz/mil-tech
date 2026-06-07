#pragma once

#include "interfaces/IBallisticsSolver.hpp"

class TableSolver : public IBallisticsSolver {
  public:
    auto calcDropParameters(const AmmoParams& ammo, float v0, float z0) -> DropParameters override;
};