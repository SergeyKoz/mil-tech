#pragma once

#include "IBallisticsSolver.hpp"

class AnalyticalSolver : public IBallisticsSolver {
  public:
    DropParameters calcDropParameters(const AmmoParams& ammo, float v0, float z0) override;

  private:
    float calcDropDistance(float t, const AmmoParams& ammo, float v0);
    float CalcDropTime(const AmmoParams& ammo, float v0, float z0);
};