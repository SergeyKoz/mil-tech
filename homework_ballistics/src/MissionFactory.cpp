#include "MissionFactory.hpp"
#include "JsonTargetProvider.hpp"
#include "AnalyticalSolver.hpp"
#include "FileConfigLoader.hpp"
#include "JsonSimulationExport.hpp"
#include "AmmoProvider.hpp"

ITargetsProvider* MissionFactory::createTargetsProvider(ProviderType providerType)
{
    if (providerType == ProviderType::JSON) {
        const char* targetsJsonPath = "homework_ballistics/data/targets.json";
        return new JsonTargetProvider(targetsJsonPath);
    }

    throw std::invalid_argument("Unsupported provider type");
}

IBallisticsSolver* MissionFactory::createBallisticsSolver(SolverType solverType)
{
    if (solverType == SolverType::ANALYTICAL) {
        return new AnalyticalSolver();
    }

    throw std::invalid_argument("Unsupported solver type");
}

IConfigLoader* MissionFactory::createConfigLoader(LoaderType loaderType)
{
    if (loaderType == LoaderType::JSON) {
        const char* ammoConfigPath = "homework_ballistics/data/ammo.json";
        AmmoProvider ammoProvider{ammoConfigPath};
        auto ammoList = ammoProvider.getAmmoList();

        const char* configPath = "homework_ballistics/data/config.json";

        return new FileConfigLoader(configPath, ammoList);
    }

    throw std::invalid_argument("Unsupported loader type");
}

ISimulationExport* MissionFactory::createSimulationExport(ExportType exportType)
{
    if (exportType == ExportType::JSON) {
        const char* jsonFilePath = "homework_ballistics/data/simulation.json";
        return new JsonSimulationExport(jsonFilePath);
    }

    throw std::invalid_argument("Unsupported export type");
}
