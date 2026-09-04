#pragma once

#include <filesystem>

#include "ballistics_simulator/scenario.hpp"

namespace ballistics_simulator {

Scenario load_scenario(const std::filesystem::path& path);

}  // namespace ballistics_simulator
