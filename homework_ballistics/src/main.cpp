#include <iostream>
#include "MissionProcessor.hpp"
#include "MissionFactory.hpp"
#include "interfaces/ITargetsProvider.hpp"
#include "interfaces/ISimulationExport.hpp"

auto main() -> int
{
    auto* ballisticsSolver = MissionFactory::createBallisticsSolver(SolverType::ANALYTICAL);
    auto* configLoader = MissionFactory::createConfigLoader(LoaderType::JSON);
    auto* targetProvider = MissionFactory::createTargetsProvider(ProviderType::JSON);
    auto* simulationExport = MissionFactory::createSimulationExport(ExportType::JSON);

    auto* mission = new MissionProcessor(*ballisticsSolver, *configLoader, *targetProvider, *simulationExport);

    try {
        mission->init();

        while (mission->hasNext()) {
            mission->step();
        }

        mission->dumpResults();
    }
    catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;

        delete mission;
        delete ballisticsSolver;
        delete configLoader;
        delete targetProvider;
        delete simulationExport;

        return 1;
    }

    delete mission;
    delete ballisticsSolver;
    delete configLoader;
    delete targetProvider;
    delete simulationExport;

    return 0;
}