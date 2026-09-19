#pragma once

#include "IntervalWorker.hpp"

class MavlinkClient;
class ThreadDronePhysics;

class AttitudeWorker : public IntervalWorker {
  public:
    AttitudeWorker(MavlinkClient* mavlinkClient, ThreadDronePhysics& dronePhysics);

    ~AttitudeWorker();

  private:
    MavlinkClient* mavlinkClient;
    ThreadDronePhysics* dronePhysics;

    auto intervalTask() -> void override;
};
