#pragma once

#include <map>
#include "interfaces/IConfigLoader.hpp"

class JsonConfigLoader : public IConfigLoader {
  public:
    JsonConfigLoader(std::string filePath, std::map<std::string, AmmoParams> ammoList);
    void load() override;
    auto getConfig() -> DroneConfig override;

  private:
    std::string configFilePath;
    DroneConfig droneConfig;
    std::map<std::string, AmmoParams> ammoList;
};