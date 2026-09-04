#pragma once

#include <memory>
#include "providers/ThreadTargetProvider.hpp"
#include "interfaces/IBallisticsSolver.hpp"
#include "interfaces/ISimulationExport.hpp"
#include "interfaces/ITargetsProvider.hpp"
#include "interfaces/IConfigLoader.hpp"

class TestFilesRepository;

enum class SolverType { ANALYTICAL, TABLE };

enum class ProviderType {
    JSON,
    CHECKER,
};

enum class ConfigLoaderType {
    JSON,
    CHECKER,
};

enum class AmmoLoaderType { JSON, STATIC, NONE };

enum class ExportType { JSON };

class MissionFactory {
  public:
    static std::unique_ptr<ITargetsProvider> createTargetsProvider(ProviderType providerType);
    static std::unique_ptr<IBallisticsSolver> createBallisticsSolver(SolverType solverType);
    static std::unique_ptr<IConfigLoader> createConfigLoader(ConfigLoaderType configLoaderType,
                                                             AmmoLoaderType ammoLoaderType,
                                                             const std::shared_ptr<TestFilesRepository> &testFilesRepository);
    static std::unique_ptr<ThreadTargetProvider> createThreadTargetsProvider(
        const DroneConfig &droneConfig, const std::shared_ptr<TestFilesRepository> &testFilesRepository);
    static std::unique_ptr<ISimulationExport> createSimulationExport(ExportType exportType,
                                                                     const std::shared_ptr<TestFilesRepository> &testFilesRepository);
};
