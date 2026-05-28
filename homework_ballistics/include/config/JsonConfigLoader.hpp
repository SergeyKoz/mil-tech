#pragma once

#include "interfaces/IConfigLoader.hpp"
#include <nlohmann/json_fwd.hpp>

using json = nlohmann::json;

class JsonConfigLoader : public IConfigLoader {
  public:
    JsonConfigLoader(std::string filePath, std::map<std::string, AmmoParams> ammoList);
    void load() override;
    auto getConfig() -> DroneConfig override;

  private:
    std::string configFilePath;
    DroneConfig droneConfig;
    std::map<std::string, AmmoParams> ammoList;
    static void resolveAmmo(const json& jsonConfig, DroneConfig& droneConfig, const std::map<std::string, AmmoParams>& ammoList);
};