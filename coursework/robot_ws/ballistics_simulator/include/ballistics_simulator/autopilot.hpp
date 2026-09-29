#pragma once

#include <memory>
#include <functional>
#include <map>
#include <stdexcept>
#include "common.hpp"
#include "interfaces/loggable_interface.hpp"

class IBallisticsSolver;
class ITargetsProvider;
class IDroneState;

namespace ballistics_simulator
{
  class TargetHit : public std::runtime_error
  {
    using std::runtime_error::runtime_error;
  };

  struct DroneConfig;
  class TargetSelector;

  class Autopilot : public ILoggable
  {

  public:
    using ControlCommandHandler = std::function<void(const ControlCommand &)>;

    Autopilot(std::unique_ptr<IBallisticsSolver> solver, std::shared_ptr<ITargetsProvider> targetsProvider);

    auto getCurrentTime() -> float;
    auto setConfig(const DroneConfig &config) -> void;
    auto setAmmo(const AmmoConfig &config) -> void;
    auto processTelemetry(DroneTelemetry &telemetry) -> void;

    void setCommandHandler(ControlCommandHandler handler);

    ~Autopilot();

  private:
    std::unique_ptr<IBallisticsSolver> solver;
    std::shared_ptr<ITargetsProvider> targetsProvider;
    std::unique_ptr<TargetSelector> targetSelector;

    float currentTime;
    DropParameters dropParams{};
    DroneConfig droneConfig;

    bool isConfigured{false};
    bool isTargetsDefined{false};
    bool isDropParametersCalculated{false};

    std::unique_ptr<DroneContext> context;
    std::map<DroneStatus, std::function<std::unique_ptr<IDroneState>()>> states;
    ControlCommandHandler controlCommandHandler = [](const ControlCommand &) {};

    auto calculateSimulationStep() -> std::unique_ptr<SimStep>;
    static auto isDroneConfigReady(const DroneConfig &droneConfig) -> bool;
    static auto isTargetHit(const DroneContext &droneContext) -> bool;
  };
}