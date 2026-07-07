#include "MissionProcessor.hpp"
#include "MissionFactory.hpp"
#include "ITargetsProvider.hpp"
#include "ISimulationExport.hpp"

auto main() -> int
{
    auto factory = new MissionFactory();
    auto ballisticsSolver = factory->createBallisticsSolver(SolverType::ANALYTICAL);
    auto configLoader = factory->createConfigLoader(LoaderType::JSON);
    auto targetProvider = factory->createTargetsProvider(ProviderType::JSON);
    auto simulationExport = factory->createSimulationExport(ExportType::JSON);

    auto mission = new MissionProcessor(*ballisticsSolver, *configLoader, *targetProvider, *simulationExport);

    mission->init();

    while (mission->hasNext()) {
        mission->step();
    }

    mission->dumpResults();

    delete mission;
    delete ballisticsSolver;
    delete configLoader;
    delete targetProvider;
    delete simulationExport;
    delete factory;

    return 0;
}