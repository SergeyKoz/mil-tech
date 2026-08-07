#pragma once

#include <string>
#include "common.hpp"

class AppConfigLoader {
  public:
    static auto load(const std::string& configFile) -> AppConfig;
};
