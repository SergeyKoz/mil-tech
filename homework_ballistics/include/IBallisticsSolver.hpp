#pragma once

struct AmmoParams;

struct DropParameters {
    float time;      // час падіння
    float distance;  // відстань, на яку відхилиться снаряд від вертикалі
};

class IBallisticsSolver {
  public:
    virtual DropParameters calcDropParameters(const AmmoParams& ammo, float v0, float z0) = 0;
    virtual ~IBallisticsSolver() = default;
};