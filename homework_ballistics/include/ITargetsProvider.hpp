#pragma once

#include "common.hpp"

struct Target {
    Coord* positions;
    Coord interpolate(float time, float timeStep)
    {
        int _current = (int)floor(time / timeStep) % 60;
        int _next = (_current + 1) % 60;
        float frac = (time - _current * timeStep) / timeStep;
        Coord current = positions[_current];
        Coord next = positions[_next];

        return {current.x + (next.x - current.x) * frac, current.y + (next.y - current.y) * frac};
    };
};

class ITargetsProvider {
  public:
    virtual void load() = 0;
    virtual int getTargetsCount() = 0;
    virtual int getTimeSteps() = 0;
    virtual Target getTarget(int index) = 0;
    virtual ~ITargetsProvider() = default;
};