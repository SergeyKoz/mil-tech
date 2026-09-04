#pragma once

#include "IntervalWorker.hpp"
#include <vector>

class I2CBus;
class II2CDevice;

class I2CDeviceListener : public IntervalWorker {
  public:
    explicit I2CDeviceListener(std::chrono::milliseconds interval);

    auto addDevice(const std::shared_ptr<II2CDevice>& device) -> void;

    ~I2CDeviceListener();

  private:
    std::mutex dataMutex;

    auto intervalTask() -> void override;

    std::vector<std::weak_ptr<II2CDevice>> devices;
};
