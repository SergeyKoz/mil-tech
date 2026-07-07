#pragma once

#include <memory>

class ITargetsProvider;
class IConfigLoader;
class IBallisticsSolver;
class ISimulationExport;

enum class SolverType { ANALYTICAL, TABLE };

enum class ProviderType { JSON };

enum class ConfigLoaderType { JSON };

enum class AmmoLoaderType { JSON, STATIC };

enum class ExportType { JSON };

class MissionFactory {
  public:
    static std::unique_ptr<ITargetsProvider> createTargetsProvider(ProviderType providerType);
    static std::unique_ptr<IBallisticsSolver> createBallisticsSolver(SolverType solverType);
    static std::unique_ptr<IConfigLoader> createConfigLoader(ConfigLoaderType configLoaderType, AmmoLoaderType ammoLoaderType);
    static std::unique_ptr<ISimulationExport> createSimulationExport(ExportType exportType);
};
