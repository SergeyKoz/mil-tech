#pragma once

#include <string>
#include <map>
#include "common.hpp"

class JsonAmmoProvider {
  public:
    JsonAmmoProvider(std::string filePath);
    auto getAmmoList() -> std::map<std::string, AmmoParams>;

  private:
    std::string configFilePath;
};