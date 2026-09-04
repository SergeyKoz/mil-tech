#include "I2CDeviceListener.hpp"
#include "interfaces/II2CDevice.hpp"

I2CDeviceListener::I2CDeviceListener(std::chrono::milliseconds interval)
    : IntervalWorker(interval)
{
}

auto I2CDeviceListener::addDevice(const std::shared_ptr<II2CDevice>& device) -> void
{
    device->ping();

    devices.push_back(device);
}

auto I2CDeviceListener::intervalTask() -> void
{
    auto it = devices.begin();
    while (it != devices.end()) {
        if (auto device = it->lock()) {
            device->renderTelemetry();
            ++it;
        }
        else {
            it = devices.erase(it);
        }
    }
}

I2CDeviceListener::~I2CDeviceListener() = default;