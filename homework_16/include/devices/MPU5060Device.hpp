#pragma once

#include "interfaces/II2CDevice.hpp"
#include <memory>
#include <span>

class I2CBus;

class MPU5060Device : public II2CDevice {
  public:
    static constexpr const char *DEVICE_NAME = "MPU5060";

    MPU5060Device();
    auto ping() -> void override;
    auto connect(const std::shared_ptr<I2CBus> &bus) -> void override;
    auto renderTelemetry() -> void override;

  private:
    struct SensorData {
        double accelX;
        double accelY;
        double accelZ;
        double temperature;
        double gyroX;
        double gyroY;
        double gyroZ;
    };

    static constexpr long DEFAULT_ADDRESS = 0x68;
    static constexpr unsigned char REG_PWR_MGMT_1 = 0x6B;
    static constexpr unsigned char REG_ACCEL_CONFIG = 0x1C;
    static constexpr unsigned char REG_GYRO_CONFIG = 0x1B;
    static constexpr unsigned char REG_ACCEL_XOUT_H = 0x3B;
    static constexpr double ACCEL_SCALE = 16384.0;  // +/- 2g
    static constexpr double GYRO_SCALE = 131.0;     // +/- 250 deg/s

    std::shared_ptr<I2CBus> bus;
    long address = DEFAULT_ADDRESS;

    auto resetBus() -> void;
    auto configure() -> void;
    auto writeRegister(unsigned char reg, unsigned char value) -> void;
    auto readRegisters(unsigned char startReg, std::span<unsigned char> buffer) -> void;
    static auto readInt16(std::span<const unsigned char> buffer, std::size_t index) -> std::int16_t;
    auto readSensorData() -> SensorData;
};
