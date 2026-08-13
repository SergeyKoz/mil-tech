#include "devices/ASD1115Device.hpp"
#include "common.hpp"
#include "I2CBus.hpp"
#include <sys/ioctl.h>
#include <array>
#include <linux/i2c-dev.h>

ASD1115Device::ASD1115Device()
    : address(0x48)
{
}

auto ASD1115Device::ping() -> void
{
    resetBus();

    uint8_t reg = 0x75;
    write(bus->getBus(), &reg, 1);
    uint8_t val = 0;
    read(bus->getBus(), &val, 1);

    LOG(DEVICE_NAME << (address == val ? " Pong " : " Is not connected"));
}

auto ASD1115Device::connect(const std::shared_ptr<I2CBus> &bus) -> void
{
    this->bus = bus;

    resetBus();

    // Configure ADS1115: AIN0 vs GND, 4.096V range, single-conversion
    // Pointer register = 0x01 (Config Register)
    // Config data = 0xC2, 0x83 (AIN0, +/-4.096V, single-shot)
    const std::array<unsigned char, 3> config = {0x01, 0xC2, 0x83};

    if (write(bus->getBus(), config.data(), config.size()) != static_cast<ssize_t>(config.size())) {
        throw std::runtime_error("Failed to configure device");
    }
}

auto ASD1115Device::resetBus() -> void
{
    if (ioctl(bus->getBus(), I2C_SLAVE, address) < 0) {  // 0x68
        throw std::runtime_error("Can't connects device to bus");
    }
}

void ASD1115Device::renderTelemetry()
{
    LOG(DEVICE_NAME << " Voltage: " << readVoltage() << "V");
}

auto ASD1115Device::readVoltage() -> double
{
    resetBus();

    unsigned char reg = 0x00;

    if (write(bus->getBus(), &reg, 1) != 1) {
        throw std::runtime_error("Failed to set pointer to conversion register.");
    }

    std::array<char, 2> buf = {0, 0};

    if (read(bus->getBus(), buf.data(), buf.size()) != static_cast<ssize_t>(buf.size())) {
        throw std::runtime_error("Failed to read.");
    }

    int rawValue = (buf[0] << 8) | buf[1];

    if (rawValue > 32767) {
        rawValue -= 65536;
    }

    return rawValue * (4.096 / 32768.0);
}
