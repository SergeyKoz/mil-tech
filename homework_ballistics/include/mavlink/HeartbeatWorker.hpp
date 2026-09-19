#pragma once

#include "IntervalWorker.hpp"
#include <arpa/inet.h>

class MavlinkClient;

class HeartbeatWorker : public IntervalWorker {
  public:
    HeartbeatWorker(MavlinkClient* mavlinkClient);

    ~HeartbeatWorker();

  private:
    MavlinkClient* mavlinkClient;
    auto intervalTask() -> void override;
};
