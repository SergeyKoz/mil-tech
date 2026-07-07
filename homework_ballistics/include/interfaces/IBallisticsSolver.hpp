#pragma once
#include "common.hpp"

struct AmmoParams;

class IBallisticsSolver {
  public:
    virtual DropParameters calcDropParameters(const AmmoParams& ammo, float v0, float z0) = 0;
    virtual ~IBallisticsSolver() = default;
};