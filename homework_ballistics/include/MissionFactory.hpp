#pragma once

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
    static ITargetsProvider* createTargetsProvider(ProviderType providerType);
    static IBallisticsSolver* createBallisticsSolver(SolverType solverType);
    static IConfigLoader* createConfigLoader(ConfigLoaderType configLoaderType, AmmoLoaderType ammoLoaderType);
    static ISimulationExport* createSimulationExport(ExportType exportType);
};
