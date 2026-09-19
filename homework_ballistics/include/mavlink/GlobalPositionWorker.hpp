#pragma once

#include "IntervalWorker.hpp"

class MavlinkClient;
class ThreadDronePhysics;
struct MavlinkConfig;

class GlobalPositionWorker : public IntervalWorker {
  public:
    GlobalPositionWorker(MavlinkConfig mavlinkConfig, MavlinkClient* mavlinkClient, ThreadDronePhysics& dronePhysics);

    ~GlobalPositionWorker();

  private:
    MavlinkConfig mavlinkConfig;
    MavlinkClient* mavlinkClient;
    ThreadDronePhysics* dronePhysics;

    auto intervalTask() -> void override;
};
