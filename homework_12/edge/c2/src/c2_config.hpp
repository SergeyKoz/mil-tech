#pragma once

#include <nlohmann/json.hpp>
#include <fstream>
#include <stdexcept>

struct C2Config {
    const uint16_t fcPort = 0.0F;
    const uint16_t asPort = 0.0F;

    static auto load(const std::string& logFile) -> C2Config
    {
        nlohmann::json cfg;
        {
            std::ifstream f(logFile);

            if (!f.is_open()) {
                throw std::invalid_argument("Cannot open" + logFile);
            }
            try {
                f >> cfg;
            } catch (const nlohmann::json::exception& e) {
                throw std::invalid_argument(std::string("invalid config: ") + e.what());
            }
        }

        return {
            .fcPort = cfg.at("fc_port").get<uint16_t>(),
            .asPort = cfg.at("as_port").get<uint16_t>(),
        };
    };
};