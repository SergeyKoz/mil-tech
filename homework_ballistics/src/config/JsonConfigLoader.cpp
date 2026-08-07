#include "config/JsonConfigLoader.hpp"
#include <nlohmann/json.hpp>

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
    droneConfig.targetTimeStep = j["simulation"]["targetTimeStep"];
    droneConfig.physicsTimeStep = j["simulation"]["physicsTimeStep"];
    droneConfig.timeScale = j["simulation"]["timeScale"];
}

void resolveAmmo(const json& jsonConfig, DroneConfig& droneConfig, const std::map<std::string, AmmoParams>& ammoList)
{
    const auto ammoName = jsonConfig["ammo"].get<const std::string>();
    auto ammoIt = ammoList.find(ammoName);

    if (ammoIt != ammoList.end()) {
        droneConfig.ammo = ammoIt->second;

        return;
    }

    throw std::runtime_error("Unable to define ammo");
}

JsonConfigLoader::JsonConfigLoader(std::ifstream configFile, std::map<std::string, AmmoParams> ammoList)
    : configFile(std::move(configFile))
    , ammoList(std::move(ammoList))
{
}

void JsonConfigLoader::load()
{
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
        throw std::runtime_error(std::string("Error parsing config file: ") + ex.what());
    }

    configFile.close();
}

auto JsonConfigLoader::getConfig() -> DroneConfig
{
    return droneConfig;
}
