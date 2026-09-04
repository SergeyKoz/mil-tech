#pragma once

#include "interfaces/II2CDevice.hpp"
#include <memory>

class I2CBus;

class ASD1115Device : public II2CDevice {
  public:
    static constexpr const char *DEVICE_NAME = "ASD1115";

    ASD1115Device();
    auto ping() -> void override;
    auto connect(const std::shared_ptr<I2CBus> &bus) -> void override;
    auto renderTelemetry() -> void override;

  private:
    long address;
    std::shared_ptr<I2CBus> bus;

    auto resetBus() -> void;
    auto readVoltage() -> double;
};
