#pragma once

#include <map>
#include "interfaces/IConfigLoader.hpp"

class CheckerConfigLoader : public IConfigLoader {
  public:
    void load() override;
    auto getConfig() -> DroneConfig override;
    auto setConfig(const DroneConfig &config) -> void;

  private:
    DroneConfig droneConfig;
};