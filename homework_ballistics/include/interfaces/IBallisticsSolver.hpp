#pragma once
#include "common.hpp"

struct AmmoParams;

class IBallisticsSolver {
  public:
    virtual void init() = 0;
    virtual DropParameters calcDropParameters(const AmmoParams& ammo, float v0, float z0) = 0;
    virtual ~IBallisticsSolver() = default;
};