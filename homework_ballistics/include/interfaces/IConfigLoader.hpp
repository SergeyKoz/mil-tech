#pragma once

#include "common.hpp"

class IConfigLoader {
  public:
    virtual void load() = 0;
    virtual DroneConfig getConfig() = 0;
    virtual ~IConfigLoader() = default;
};