#include "devices/MPU5060Device.hpp"
#include "common.hpp"
#include "I2CBus.hpp"
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <array>

MPU5060Device::MPU5060Device()
    : address(0x68)
{
}

auto MPU5060Device::ping() -> void
{
    resetBus();

    uint8_t reg = 0x75;
    write(bus->getBus(), &reg, 1);
    uint8_t val = 0;
    read(bus->getBus(), &val, 1);

    LOG(DEVICE_NAME << (address == val ? " Pong " : " Is not connected"));
}

auto MPU5060Device::connect(const std::shared_ptr<I2CBus> &bus) -> void
{
    this->bus = bus;

    resetBus();
    configure();
}

auto MPU5060Device::resetBus() -> void
{
    if (ioctl(bus->getBus(), I2C_SLAVE, address) < 0) {  // 0x68
        throw std::runtime_error("Can't connects device to bus");
    }
}

void MPU5060Device::renderTelemetry()
{
    const auto data = readSensorData();

    LOG(DEVICE_NAME << " Accel[g]: " << "x=" << data.accelX << " y=" << data.accelY << " z=" << data.accelZ << " Temp[C]: "
                    << data.temperature << " Gyro[deg/s]: " << "x=" << data.gyroX << " y=" << data.gyroY << " z=" << data.gyroZ);
}

auto MPU5060Device::configure() -> void
{
    // Wake up MPU-6050. By default it can be in sleep mode.
    writeRegister(REG_PWR_MGMT_1, 0x00);

    // Accelerometer range:
    // 0x00 = +/- 2g
    // 0x08 = +/- 4g
    // 0x10 = +/- 8g
    // 0x18 = +/- 16g
    writeRegister(REG_ACCEL_CONFIG, 0x00);

    // Gyroscope range:
    // 0x00 = +/- 250 deg/s
    // 0x08 = +/- 500 deg/s
    // 0x10 = +/- 1000 deg/s
    // 0x18 = +/- 2000 deg/s
    writeRegister(REG_GYRO_CONFIG, 0x00);
}

auto MPU5060Device::writeRegister(unsigned char reg, unsigned char value) -> void
{
    const std::array<unsigned char, 2> data = {reg, value};

    if (write(bus->getBus(), data.data(), data.size()) != static_cast<ssize_t>(data.size())) {
        throw std::runtime_error("Failed to write MPU5060 register");
    }
}

auto MPU5060Device::readRegisters(unsigned char startReg, std::span<unsigned char> buffer) -> void
{
    const std::array<unsigned char, 1> reg = {startReg};

    if (write(bus->getBus(), reg.data(), reg.size()) != static_cast<ssize_t>(reg.size())) {
        throw std::runtime_error("Failed to set MPU5060 register pointer");
    }

    if (read(bus->getBus(), buffer.data(), buffer.size()) != static_cast<ssize_t>(buffer.size())) {
        throw std::runtime_error("Failed to read MPU5060 registers");
    }
}

auto MPU5060Device::readInt16(std::span<const unsigned char> buffer, std::size_t index) -> std::int16_t
{
    return static_cast<std::int16_t>((buffer[index] << 8) | buffer[index + 1]);
}

auto MPU5060Device::readSensorData() -> SensorData
{
    resetBus();

    std::array<unsigned char, 14> buffer = {};

    readRegisters(REG_ACCEL_XOUT_H, buffer);

    const auto rawAccelX = readInt16(buffer, 0);
    const auto rawAccelY = readInt16(buffer, 2);
    const auto rawAccelZ = readInt16(buffer, 4);
    const auto rawTemp = readInt16(buffer, 6);
    const auto rawGyroX = readInt16(buffer, 8);
    const auto rawGyroY = readInt16(buffer, 10);
    const auto rawGyroZ = readInt16(buffer, 12);

    return SensorData{
        .accelX = rawAccelX / ACCEL_SCALE,
        .accelY = rawAccelY / ACCEL_SCALE,
        .accelZ = rawAccelZ / ACCEL_SCALE,
        .temperature = rawTemp / 340.0 + 36.53,
        .gyroX = rawGyroX / GYRO_SCALE,
        .gyroY = rawGyroY / GYRO_SCALE,
        .gyroZ = rawGyroZ / GYRO_SCALE,
    };
}