#include <memory>
#include <string>
#include "MissionFactory.hpp"
#include "providers/JsonTargetProvider.hpp"
#include "providers/ThreadTargetProvider.hpp"
#include "providers/CheckerTargetProvider.hpp"
#include "solvers/AnalyticalSolver.hpp"
#include "solvers/TableSolver.hpp"
#include "config/JsonConfigLoader.hpp"
#include "config/CheckerConfigLoader.hpp"
#include "export/JsonSimulationExport.hpp"
#include "providers/JsonAmmoProvider.hpp"
#include "interfaces/IConfigLoader.hpp"
#include "interfaces/ITargetsProvider.hpp"
#include "interfaces/IBallisticsSolver.hpp"
#include "TestFilesRepository.hpp"

auto MissionFactory::createTargetsProvider(ProviderType providerType) -> std::unique_ptr<ITargetsProvider>
{
    switch (providerType) {
        case ProviderType::JSON: {
            const auto *targetsJsonPath = "homework_ballistics/data/targets.json";

            return std::make_unique<JsonTargetProvider>(targetsJsonPath);
        }
        case ProviderType::CHECKER:
            return std::make_unique<CheckerTargetProvider>();
    };

    throw std::invalid_argument("Unsupported provider type");
}

auto MissionFactory::createThreadTargetsProvider(const DroneConfig &droneConfig,
                                                 const std::shared_ptr<TestFilesRepository> &testFilesRepository)
    -> std::unique_ptr<ThreadTargetProvider>
{
    return std::make_unique<ThreadTargetProvider>(testFilesRepository->getTargetsFile(), droneConfig);
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

auto MissionFactory::createConfigLoader(ConfigLoaderType configLoaderType,
                                        AmmoLoaderType ammoLoaderType,
                                        const std::shared_ptr<TestFilesRepository> &testFilesRepository) -> std::unique_ptr<IConfigLoader>
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
        case AmmoLoaderType::NONE:
            break;
    };

    switch (configLoaderType) {
        case ConfigLoaderType::JSON: {
            return std::make_unique<JsonConfigLoader>(testFilesRepository->getConfigFile(), ammoList);
        }
        case ConfigLoaderType::CHECKER:
            return std::make_unique<CheckerConfigLoader>();
    };

    throw std::invalid_argument("Unsupported loader type");
}

auto MissionFactory::createSimulationExport(ExportType exportType, const std::shared_ptr<TestFilesRepository> &testFilesRepository)
    -> std::unique_ptr<ISimulationExport>
{
    if (exportType == ExportType::JSON) {
        return std::make_unique<JsonSimulationExport>(testFilesRepository->putSimulationFile());
    }

    throw std::invalid_argument("Unsupported export type");
}
