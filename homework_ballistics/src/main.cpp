#include <iostream>
#include <memory>
#include "MissionFactory.hpp"
#include "TestFilesStorage.hpp"
#include "ThreadDronePhysics.hpp"
#include "ThreadMissionProcessor.hpp"
#include "TestFilesRepository.hpp"
#include "config/AppConfigLoader.hpp"

auto main() -> int
{
    constexpr std::array<TestCode, 10> tests = {TestCode::T1,
                                                TestCode::T2,
                                                TestCode::T3,
                                                TestCode::T4,
                                                TestCode::T5,
                                                TestCode::T6,
                                                TestCode::T7,
                                                TestCode::T8,
                                                TestCode::T9,
                                                TestCode::T10};

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