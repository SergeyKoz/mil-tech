#include "mavlink/HeartbeatWorker.hpp"
#include "mavlink/MavlinkClient.hpp"
#include <cmath>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

HeartbeatWorker::HeartbeatWorker(MavlinkClient* mavlinkClient)
    : IntervalWorker(std::chrono::milliseconds(1000), 1)
    , mavlinkClient(mavlinkClient)
{
}

auto HeartbeatWorker::intervalTask() -> void
{
    mavlink_message_t message{};

    mavlink_msg_heartbeat_pack(mavlinkClient->getSystemId(),
                               MavlinkClient::COMP_ID,
                               &message,
                               MAV_TYPE_QUADROTOR,
                               MAV_AUTOPILOT_GENERIC,
                               MAV_MODE_FLAG_MANUAL_INPUT_ENABLED | MAV_MODE_FLAG_MANUAL_INPUT_ENABLED,
                               0,
                               MAV_STATE_STANDBY);  // MAV_STATE_ACTIVE

    mavlinkClient->sendMessage(message);
}

HeartbeatWorker::~HeartbeatWorker() = default;