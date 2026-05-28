#include <string>
#include "MissionFactory.hpp"
#include "providers/JsonTargetProvider.hpp"
#include "solvers/AnalyticalSolver.hpp"
#include "config/JsonConfigLoader.hpp"
#include "export/JsonSimulationExport.hpp"
#include "providers/AmmoProvider.hpp"

auto MissionFactory::createTargetsProvider(ProviderType providerType) -> ITargetsProvider *
{
    if (providerType == ProviderType::JSON) {
        const auto *targetsJsonPath = "homework_ballistics/data/targets.json";

        return new JsonTargetProvider(targetsJsonPath);
    }

    throw std::invalid_argument("Unsupported provider type");
}

auto MissionFactory::createBallisticsSolver(SolverType solverType) -> IBallisticsSolver *
{
    if (solverType == SolverType::ANALYTICAL) {
        return new AnalyticalSolver();
    }

    throw std::invalid_argument("Unsupported solver type");
}

auto MissionFactory::createConfigLoader(LoaderType loaderType) -> IConfigLoader *
{
    if (loaderType == LoaderType::JSON) {
        const auto *ammoConfigPath = "homework_ballistics/data/ammo.json";
        AmmoProvider ammoProvider{ammoConfigPath};
        auto ammoList = ammoProvider.getAmmoList();

        const auto *configPath = "homework_ballistics/data/config.json";

        return new JsonConfigLoader(configPath, ammoList);
    }

    throw std::invalid_argument("Unsupported loader type");
}

auto MissionFactory::createSimulationExport(ExportType exportType) -> ISimulationExport *
{
    if (exportType == ExportType::JSON) {
        const auto *jsonFilePath = "homework_ballistics/data/simulation.json";
        return new JsonSimulationExport(jsonFilePath);
    }

    throw std::invalid_argument("Unsupported export type");
}
