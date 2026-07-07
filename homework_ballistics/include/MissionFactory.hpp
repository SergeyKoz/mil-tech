#pragma once

class ITargetsProvider;
class IConfigLoader;
class IBallisticsSolver;
class ISimulationExport;

enum class SolverType { ANALYTICAL };

enum class ProviderType { JSON };

enum class LoaderType { JSON };

enum class ExportType { JSON };

class MissionFactory {
  public:
    static ITargetsProvider* createTargetsProvider(ProviderType providerType);
    static IBallisticsSolver* createBallisticsSolver(SolverType solverType);
    static IConfigLoader* createConfigLoader(LoaderType loaderType);
    static ISimulationExport* createSimulationExport(ExportType exportType);
};
