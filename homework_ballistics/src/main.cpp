#include <iostream>
#include <memory>
#include "MissionProcessor.hpp"
#include "MissionFactory.hpp"
#include "interfaces/ITargetsProvider.hpp"
#include "interfaces/ISimulationExport.hpp"

auto main() -> int
{
    try {
        auto ballisticsSolver = MissionFactory::createBallisticsSolver(SolverType::TABLE);
        auto configLoader = MissionFactory::createConfigLoader(ConfigLoaderType::JSON, AmmoLoaderType::STATIC);
        auto targetProvider = MissionFactory::createTargetsProvider(ProviderType::JSON);
        auto simulationExport = MissionFactory::createSimulationExport(ExportType::JSON);

        auto mission = std::make_unique<MissionProcessor>(
            std::move(ballisticsSolver), std::move(configLoader), std::move(targetProvider), std::move(simulationExport));

        mission->init();

        while (mission->hasNext()) {
            mission->step();
        }

        mission->dumpResults();
    }
    catch (const std::runtime_error& ex) {
        std::cerr << ex.what() << '\n';

        return 1;
    }
    catch (const std::invalid_argument& ex) {
        std::cerr << ex.what() << '\n';

        return 1;
    }
    catch (const std::exception& ex) {
        std::cerr << ex.what() << '\n';

        return 1;
    }

    return 0;
}