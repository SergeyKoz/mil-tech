#pragma once

#include <map>
#include <fstream>
#include "interfaces/IConfigLoader.hpp"

class JsonConfigLoader : public IConfigLoader {
  public:
    JsonConfigLoader(std::ifstream configFile, std::map<std::string, AmmoParams> ammoList);
    void load() override;
    auto getConfig() -> DroneConfig override;

  private:
    std::string configFilePath;
    std::ifstream configFile;
    DroneConfig droneConfig;
    std::map<std::string, AmmoParams> ammoList;
};