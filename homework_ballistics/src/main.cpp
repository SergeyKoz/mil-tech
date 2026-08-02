#include <iostream>
#include "MissionFactory.hpp"
#include "ThreadDronePhysics.hpp"
#include "ThreadMissionProcessor.hpp"

auto main() -> int
{
    try {
        auto ballisticsSolver = MissionFactory::createBallisticsSolver(SolverType::TABLE);
        auto configLoader = MissionFactory::createConfigLoader(ConfigLoaderType::JSON, AmmoLoaderType::STATIC);
        auto simulationExport = MissionFactory::createSimulationExport(ExportType::JSON);

        configLoader->load();
        auto droneConfig = configLoader->getConfig();

        auto targetProvider = MissionFactory::createThreadTargetsProvider(droneConfig);
        targetProvider->load();

        auto dronePhysics = std::make_unique<ThreadDronePhysics>(droneConfig);

        auto missionProcessor = std::make_unique<ThreadMissionProcessor>(
            std::move(ballisticsSolver), droneConfig, *dronePhysics.get(), *targetProvider, std::move(simulationExport));

        missionProcessor->init();

        missionProcessor->start();
        dronePhysics->start();
        targetProvider->start();

        while (!missionProcessor->isThreadReady() || !dronePhysics->isThreadReady() || !targetProvider->isThreadReady()) {
            std::this_thread::yield();
        }

        missionProcessor->wait();
        dronePhysics->stop();
        targetProvider->stop();

        missionProcessor->dumpResults();
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