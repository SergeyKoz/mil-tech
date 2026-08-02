#include <memory>
#include <string>
#include "MissionFactory.hpp"
#include "providers/JsonTargetProvider.hpp"
#include "providers/ThreadTargetProvider.hpp"
#include "solvers/AnalyticalSolver.hpp"
#include "solvers/TableSolver.hpp"
#include "config/JsonConfigLoader.hpp"
#include "export/JsonSimulationExport.hpp"
#include "providers/JsonAmmoProvider.hpp"
#include "interfaces/IConfigLoader.hpp"
#include "interfaces/ITargetsProvider.hpp"
#include "interfaces/IBallisticsSolver.hpp"

auto MissionFactory::createTargetsProvider(ProviderType providerType) -> std::unique_ptr<ITargetsProvider>
{
    if (providerType == ProviderType::JSON) {
        const auto *targetsJsonPath = "homework_ballistics/data/targets.json";

        return std::make_unique<JsonTargetProvider>(targetsJsonPath);
    }

    throw std::invalid_argument("Unsupported provider type");
}

auto MissionFactory::createThreadTargetsProvider(const DroneConfig &droneConfig) -> std::unique_ptr<ThreadTargetProvider>
{
    const auto *targetsJsonPath = "homework_ballistics/data/targets.json";

    return std::make_unique<ThreadTargetProvider>(targetsJsonPath, droneConfig);
}

auto MissionFactory::createBallisticsSolver(SolverType solverType) -> std::unique_ptr<IBallisticsSolver>
{
    switch (solverType) {
        case SolverType::ANALYTICAL:
            return std::make_unique<AnalyticalSolver>();
        case SolverType::TABLE:
            const auto *ballicticTableFile = "homework_ballistics/data/ballistic_table.txt";

            return std::make_unique<TableSolver>(ballicticTableFile);
    };

    throw std::invalid_argument("Unsupported solver type");
}

auto MissionFactory::createConfigLoader(ConfigLoaderType configLoaderType, AmmoLoaderType ammoLoaderType) -> std::unique_ptr<IConfigLoader>
{
    std::map<std::string, AmmoParams> ammoList;

    switch (ammoLoaderType) {
        case AmmoLoaderType::JSON: {
            const auto *ammoConfigPath = "homework_ballistics/data/ammo.json";
            JsonAmmoProvider ammoProvider{ammoConfigPath};
            ammoList = ammoProvider.getAmmoList();

            break;
        }
        case AmmoLoaderType::STATIC:
            ammoList = {
                {"VOG-17", {.name = "VOG-17", .mass = 0.35F, .drag = 0.004F, .lift = 0.F}},
                {"M67", {.name = "M67", .mass = 0.6F, .drag = 0.005F, .lift = 0.F}},
                {"RKG-3", {.name = "RKG-3", .mass = 1.2F, .drag = 0.007F, .lift = 0.F}},
                {"GLIDING-VOG", {.name = "GLIDING-VOG", .mass = 0.45F, .drag = 0.005F, .lift = 0.005F}},
                {"GLIDING-RKG", {.name = "GLIDING-RKG", .mass = 1.4F, .drag = 0.007F, .lift = 0.005F}},
            };

            break;
    };

    if (configLoaderType == ConfigLoaderType::JSON) {
        const auto *configPath = "homework_ballistics/data/config.json";

        return std::make_unique<JsonConfigLoader>(configPath, ammoList);
    }

    throw std::invalid_argument("Unsupported loader type");
}

auto MissionFactory::createSimulationExport(ExportType exportType) -> std::unique_ptr<ISimulationExport>
{
    if (exportType == ExportType::JSON) {
        const auto *jsonFilePath = "homework_ballistics/data/simulation.json";

        return std::make_unique<JsonSimulationExport>(jsonFilePath);
    }

    throw std::invalid_argument("Unsupported export type");
}
