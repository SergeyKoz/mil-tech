#pragma once

#include "IConfigLoader.hpp"
#include <nlohmann/json_fwd.hpp>

using json = nlohmann::json;

class FileConfigLoader : public IConfigLoader {
  public:
    FileConfigLoader(const char* filePath, AmmoList ammoList);
    void load() override;
    DroneConfig getConfig() override;

  private:
    std::string configFilePath;
    DroneConfig droneConfig;
    AmmoList ammoList;
    void resolveAmmo(const json& jsonConfig, DroneConfig& droneConfig, const AmmoList& ammoList);
    ~FileConfigLoader() override;
};