
#include "AmmoProvider.hpp"
#include <cstddef>
#include <nlohmann/json.hpp>
#include <string>
#include <fstream>

using json = nlohmann::json;

AmmoProvider::AmmoProvider(const char* filePath)
  : configFilePath(filePath)
{
}

AmmoList AmmoProvider::getAmmoList()
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
    AmmoParams* ammoList = new AmmoParams[outCount];

    for (size_t i = 0; i < outCount; i++) {
        const char* ammoName = ammoConfig[i]["name"].get_ref<const std::string&>().c_str();
        ammoList[i] = {.name = "", .mass = ammoConfig[i]["mass"], .drag = ammoConfig[i]["drag"], .lift = ammoConfig[i]["lift"]};
        std::strncpy(ammoList[i].name, ammoName, 31);
        ammoList[i].name[31] = '\0';
    }

    ammoFile.close();

    return {.count = static_cast<int>(outCount), .ammo = ammoList};
};