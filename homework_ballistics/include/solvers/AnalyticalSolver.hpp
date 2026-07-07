#pragma once

#include "interfaces/IBallisticsSolver.hpp"

class AnalyticalSolver : public IBallisticsSolver {
  public:
    auto calcDropParameters(const AmmoParams& ammo, float v0, float z0) -> DropParameters override;

  private:
    static auto calcDropDistance(float t, const AmmoParams& ammo, float v0) -> float;
    static auto CalcDropTime(const AmmoParams& ammo, float v0, float z0) -> float;
};