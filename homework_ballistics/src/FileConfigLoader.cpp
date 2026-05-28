
#include "FileConfigLoader.hpp"
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

void from_json(const json& j, DroneConfig& droneConfig)
{
    droneConfig.startPos = {j["drone"]["position"]["x"], j["drone"]["position"]["y"]};
    droneConfig.altitude = j["drone"]["altitude"];
    droneConfig.initialDir = j["drone"]["initialDirection"];
    droneConfig.attackSpeed = j["drone"]["attackSpeed"];
    droneConfig.accelerationPath = j["drone"]["accelerationPath"];
    droneConfig.arrayTimeStep = j["targetArrayTimeStep"];
    droneConfig.simTimeStep = j["simulation"]["timeStep"];
    droneConfig.hitRadius = j["simulation"]["hitRadius"];
    droneConfig.turnThreshold = j["drone"]["turnThreshold"];
    droneConfig.angularSpeed = j["drone"]["angularSpeed"];
}

FileConfigLoader::FileConfigLoader(const char* filePath, AmmoList ammoList)
    : configFilePath(filePath)
    , ammoList(ammoList)
{
}

void FileConfigLoader::load()
{
    std::ifstream configFile{configFilePath};

    if (!configFile.is_open()) {
        throw std::runtime_error("Unable to open config file");
    }

    json jsonConfig;

    try {
        configFile >> jsonConfig;
        droneConfig = jsonConfig.get<DroneConfig>();
        resolveAmmo(jsonConfig, droneConfig, ammoList);
    }
    catch (const std::exception& ex) {
    }

    configFile.close();
}

void FileConfigLoader::resolveAmmo(const json& jsonConfig, DroneConfig& droneConfig, const AmmoList& ammoList)
{
    const char* ammoName = jsonConfig["ammo"].get_ref<const std::string&>().c_str();

    for (int i = 0; i < ammoList.count; i++) {
        if (strcmp(ammoList.ammo[i].name, ammoName) == 0) {
            droneConfig.ammo = ammoList.ammo[i];

            return;
        }
    }

    throw std::runtime_error("Unable to define ammo");
}

DroneConfig FileConfigLoader::getConfig()
{
    return droneConfig;
}

FileConfigLoader::~FileConfigLoader()
{
    delete[] ammoList.ammo;
}
