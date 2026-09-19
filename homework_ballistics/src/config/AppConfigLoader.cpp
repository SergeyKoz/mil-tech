#include "config/AppConfigLoader.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>
#include "common.hpp"

using json = nlohmann::json;

void from_json(const json& j, AppConfig& appConfig)
{
    appConfig.studentId = j["studentId"].get<std::string>();
    appConfig.testsRepositoryConfig = TestsRepositoryConfig{.path = j["testsRepositoryConfig"]["path"].get<std::string>()};
    appConfig.testsStorageServer = TestsStorageConfig{.url = j["testsStorageServer"]["url"].get<std::string>(),
                                                      .apiKey = j["testsStorageServer"]["apiKey"].get<std::string>(),
                                                      .connectionTimeout = j["testsStorageServer"]["connectionTimeout"].get<long>(),
                                                      .readTimeout = j["testsStorageServer"]["readTimeout"].get<long>(),
                                                      .writeTimeout = j["testsStorageServer"]["writeTimeout"].get<long>()};

    appConfig.mavlinkConfig = MavlinkConfig{
        .systemId = j["mavlink"]["systemId"].get<int>(),
        .qgsConfig = QgsConfig{.ip = j["mavlink"]["qgs"]["ip"].get<std::string>(),
                               .port = j["mavlink"]["qgs"]["port"].get<int>(),
                               .originLocation = Location{.latitude = j["mavlink"]["qgs"]["originLocation"]["latitude"].get<float>(),
                                                          .longtitude = j["mavlink"]["qgs"]["originLocation"]["longtitude"].get<float>()}}};
}

auto AppConfigLoader::load(const std::string& configFile) -> AppConfig
{
    std::ifstream config{"homework_ballistics/" + configFile};

    if (!config.is_open()) {
        throw std::runtime_error("Unable to open config file");
    }

    json jsonConfig;
    AppConfig appConfig;

    try {
        config >> jsonConfig;
        appConfig = jsonConfig.get<AppConfig>();
    }
    catch (const std::exception& ex) {
        throw std::runtime_error(std::string("Error parsing app settings file: ") + ex.what());
    }

    config.close();

    return appConfig;
}
