#pragma once

#include <string>
#include <map>
#include "common.hpp"

class AmmoProvider {
  public:
    AmmoProvider(std::string filePath);
    auto getAmmoList() -> std::map<std::string, AmmoParams>;

  private:
    std::string configFilePath;
};