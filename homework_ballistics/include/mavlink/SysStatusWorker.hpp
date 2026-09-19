#pragma once

#include "IntervalWorker.hpp"

class MavlinkClient;

class SysStatusWorker : public IntervalWorker {
  public:
    SysStatusWorker(MavlinkClient* mavlinkClient);

    ~SysStatusWorker();

  private:
    MavlinkClient* mavlinkClient;
    auto intervalTask() -> void override;
};
