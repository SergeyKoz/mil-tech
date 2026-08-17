#include <iostream>
#include <memory>
#include "MissionFactory.hpp"
#include "TestFilesStorage.hpp"
#include "ThreadDronePhysics.hpp"
#include "ThreadMissionProcessor.hpp"
#include "TestFilesRepository.hpp"
#include "config/AppConfigLoader.hpp"
#include "mavlink/MavlinkClient.hpp"
#include "mavlink/MavlinkTargetProcessor.hpp"

auto main() -> int
{
    constexpr std::array<TestCode, 10> tests = {TestCode::T1};

    try {
        auto appConfig = AppConfigLoader::load("app_config.json");
        auto testFilesRepository = std::make_shared<TestFilesRepository>(appConfig.testsRepositoryConfig.path);

        for (const auto& testCode : tests) {
            testFilesRepository->setTest(testCode);

            if (!testFilesRepository->hasSimulationFile()) {
                auto configLoader = MissionFactory::createConfigLoader(ConfigLoaderType::JSON, AmmoLoaderType::STATIC, testFilesRepository);
                auto simulationExport = MissionFactory::createSimulationExport(ExportType::JSON, testFilesRepository);
                auto ballisticsSolver = MissionFactory::createBallisticsSolver(SolverType::TABLE);

                configLoader->load();
                auto droneConfig = configLoader->getConfig();

                auto targetProvider = MissionFactory::createThreadTargetsProvider(droneConfig, testFilesRepository);
                targetProvider->load();

                auto dronePhysics = std::make_unique<ThreadDronePhysics>(droneConfig);
                auto mavlinkClient = std::make_shared<MavlinkClient>(appConfig.mavlinkConfig, *dronePhysics.get());

                auto missionProcessor = std::make_unique<ThreadMissionProcessor>(
                    std::move(ballisticsSolver), droneConfig, *dronePhysics.get(), *targetProvider, std::move(simulationExport));

                missionProcessor->init();
                missionProcessor->addTargetProcessor(
                    std::make_unique<MavlinkTargetProcessor>(appConfig.mavlinkConfig, mavlinkClient.get()));

                missionProcessor->start();
                dronePhysics->start();
                targetProvider->start();
                mavlinkClient->start();

                while (!missionProcessor->isThreadReady() || !dronePhysics->isThreadReady() || !targetProvider->isThreadReady() ||
                       !mavlinkClient->isThreadReady()) {
                    std::this_thread::yield();
                }

                missionProcessor->wait();
                dronePhysics->stop();
                targetProvider->stop();
                mavlinkClient->stop();

                missionProcessor->dumpResults();
            }

            // send simulation file to the server
            auto testFilesStorage = std::make_unique<TestFilesStorage>(appConfig.studentId, appConfig.testsStorageServer);

            testFilesStorage->send(testCode, testFilesRepository->getSimulationFile());
        }
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