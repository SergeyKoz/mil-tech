#pragma once

#include <memory>
#include "providers/ThreadTargetProvider.hpp"
#include "interfaces/IBallisticsSolver.hpp"
#include "interfaces/ISimulationExport.hpp"
#include "interfaces/ITargetsProvider.hpp"
#include "interfaces/IConfigLoader.hpp"

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
    static std::unique_ptr<IConfigLoader> createConfigLoader(ConfigLoaderType configLoaderType, AmmoLoaderType ammoLoaderType);
    static std::unique_ptr<ThreadTargetProvider> createThreadTargetsProvider(const DroneConfig &droneConfig);
    static std::unique_ptr<ISimulationExport> createSimulationExport(ExportType exportType);
};
