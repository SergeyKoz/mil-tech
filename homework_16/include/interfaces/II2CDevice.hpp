#pragma once

#include <memory>

class I2CBus;

class II2CDevice {
  public:
    virtual auto ping() -> void = 0;
    virtual auto connect(const std::shared_ptr<I2CBus> &bus) -> void = 0;
    virtual auto renderTelemetry() -> void = 0;
    virtual ~II2CDevice() = default;
};