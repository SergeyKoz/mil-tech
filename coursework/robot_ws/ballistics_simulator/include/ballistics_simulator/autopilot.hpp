#pragma once

#include <memory>
// #include <functional>
// #include <map>
#include <stdexcept>
// #include "interfaces/IUartListener.hpp"
#include "common.hpp"
#include "interfaces/loggable_interface.hpp"

// struct gpiod_chip;
// struct gpiod_line_request;
class IBallisticsSolver;
// class IConfigLoader;
class ITargetsProvider;
// class IDroneState;
// class RpiConfigLoader;
// class TargetSelector;
// class RpiCheckerGPIO;
// class RpiCheckerUART;

namespace ballistics_simulator
{
  class TargetHit : public std::runtime_error
  {
    using std::runtime_error::runtime_error;
  };

  struct DroneConfig;
  class TargetSelector;
  // struct DroneContext;

  class Autopilot : public ILoggable
  { //: public IUartListener {
  public:
    Autopilot(std::unique_ptr<IBallisticsSolver> solver, std::shared_ptr<ITargetsProvider> targetsProvider);
    // Autopilot(std::unique_ptr<IBallisticsSolver> solver,
    //           std::unique_ptr<IConfigLoader> configLoader,
    //           std::unique_ptr<ITargetsProvider> targetProvider,
    //           std::unique_ptr<RpiCheckerGPIO> rpiCheckerGPIO,
    //           std::unique_ptr<RpiCheckerUART> rpiCheckerUART);
    // Autopilot();

    auto getCurrentTime() -> float;

    auto setConfig(const DroneConfig &config) -> void;
    auto setAmmo(const AmmoConfig &config) -> void;
    auto processTelemetry(DroneTelemetry &telemetry) -> void;
    // auto setTarget(const DroneTelemetry &telemetry) -> void;

    // auto init() -> void;
    // auto start() -> void;
    // auto mission() -> void;
    // auto drop() -> void;

    // auto updateTelemetry(const dlink::Telemetry &telemetry) -> void override;
    // auto updateTargetPosition(const dlink::TargetPos &targetPosition) -> void override;
    // auto updateAmmoConfig(const dlink::AmmoCfg &ammoConfig) -> void override;
    // auto updateResult(const dlink::Result &result) -> void override;
    // auto updateDroneConfig(const dlink::DroneCfg &droneConfig) -> void override;
    // auto updateControl(const dlink::Control &control) -> void override;

    ~Autopilot();

  private:
    std::unique_ptr<IBallisticsSolver> solver;
    // std::unique_ptr<IConfigLoader> configLoader;
    std::shared_ptr<ITargetsProvider> targetsProvider;
    std::unique_ptr<TargetSelector> targetSelector;
    // std::unique_ptr<RpiCheckerGPIO> rpiCheckerGPIO;
    // std::unique_ptr<RpiCheckerUART> rpiCheckerUART;

    float currentTime;
    DropParameters dropParams{};
    DroneConfig droneConfig;

    bool isConfigured{false};
    bool isTargetsDefined{false};
    bool isDropParametersCalculated{false};

    // int uart;

    std::unique_ptr<DroneContext> context;
    // std::map<DroneStatus, std::function<std::unique_ptr<IDroneState>(TargetSelector &)>> states;

    auto calculateSimulationStep() -> std::unique_ptr<SimStep>;
    static auto isDroneConfigReady(const DroneConfig &droneConfig) -> bool;
    static auto isTargetHit(const DroneContext &droneContext) -> bool;
  };
}