
#include "providers/AmmoProvider.hpp"
#include <cstddef>
#include <nlohmann/json.hpp>
#include <string>
#include <fstream>
#include <map>

using json = nlohmann::json;

AmmoProvider::AmmoProvider(std::string filePath)
    : configFilePath(std::move(filePath))
{
}

auto AmmoProvider::getAmmoList() -> std::map<std::string, AmmoParams>
{
    std::ifstream ammoFile{configFilePath};

    if (!ammoFile.is_open()) {
        throw std::runtime_error("Unable to open ammo file");
    }

    json ammoConfig = json::parse(ammoFile);

    if (!ammoConfig.is_array()) {
        throw std::runtime_error("Invalid ammo configuration format");
    }

    auto outCount = ammoConfig.size();
    std::map<std::string, AmmoParams> ammoList;

    for (size_t i = 0; i < outCount; i++) {
        const auto ammoName = ammoConfig[i]["name"].get<const std::string>();

        ammoList[ammoName] = {
            .name = ammoName, .mass = ammoConfig[i]["mass"], .drag = ammoConfig[i]["drag"], .lift = ammoConfig[i]["lift"]};
    }

    ammoFile.close();

    // return {.ammo = ammoList};
    return ammoList;
};