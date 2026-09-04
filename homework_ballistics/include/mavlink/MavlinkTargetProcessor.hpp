#pragma once

#include "interfaces/ITargetProcessor.hpp"

class MavlinkClient;
struct MavlinkConfig;

class MavlinkTargetProcessor : public ITargetProcessor {
  public:
    MavlinkTargetProcessor(MavlinkConfig mavlinkConfig, MavlinkClient* mavlinkClient);

    auto processTarget(const SelectedTarget& target, const DroneTelemetry& droneTelemetry) -> void override;

  private:
    MavlinkConfig mavlinkConfig;
    MavlinkClient* mavlinkClient;
};