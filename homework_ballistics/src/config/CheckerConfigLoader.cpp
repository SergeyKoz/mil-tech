#include "config/CheckerConfigLoader.hpp"

void CheckerConfigLoader::load() {}

auto CheckerConfigLoader::setConfig(const DroneConfig &config) -> void
{
    droneConfig = config;
}

auto CheckerConfigLoader::getConfig() -> DroneConfig
{
    return droneConfig;
}