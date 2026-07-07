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
    ITargetsProvider* createTargetsProvider(ProviderType providerType);
    IBallisticsSolver* createBallisticsSolver(SolverType solverType);
    IConfigLoader* createConfigLoader(LoaderType loaderType);
    ISimulationExport* createSimulationExport(ExportType exportType);
};
