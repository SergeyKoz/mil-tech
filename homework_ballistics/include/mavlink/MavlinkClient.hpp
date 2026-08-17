#pragma once

#include "common.hpp"
#include "mavlink/AttitudeWorker.hpp"
#include "mavlink/GlobalPositionWorker.hpp"
#include "mavlink/HeartbeatWorker.hpp"
#include <common/mavlink.h>
#include <common/mavlink.h>

class ThreadDronePhysics;
class SysStatusWorker;
class GlobalPositionWorker;
class AttitudeWorker;
struct MavlinkConfig;

struct MavlinkTransport {
    int resource;
    sockaddr_in addresss{};
};

class MavlinkClient {
  public:
    static constexpr const uint8_t COMP_ID = MAV_COMP_ID_AUTOPILOT1;
    static constexpr const int COMMAND_ACK_MAX_ATTEMPTS = 5;
    static constexpr const int COMMAND_ACK_TIMEOUT_MS = 1000;

    MavlinkClient(const MavlinkConfig& mavlinkConfig, ThreadDronePhysics& dronePhysics);

    void start();
    void wait();
    bool isThreadReady() const;
    void stop();

    auto getSystemId() const -> int;
    auto getTransport() const -> MavlinkTransport;
    auto sendMessage(const mavlink_message_t& message) -> void;
    auto sendMessageWithAck(const mavlink_message_t& message) -> bool;

    virtual ~MavlinkClient();

  private:
    MavlinkConfig mavlinkConfig;
    MavlinkTransport mavlinkTransport;
    std::unique_ptr<HeartbeatWorker> heartbeatWorker;
    std::unique_ptr<SysStatusWorker> sysStatusWorker;
    std::unique_ptr<GlobalPositionWorker> globalPositionWorker;
    std::unique_ptr<AttitudeWorker> attitudeWorker;
    std::thread receiveThread;
    std::atomic<bool> isRunning{false};

    std::mutex commandAckMutex;
    std::condition_variable commandAckCondition;
    uint16_t expectedAckCommand{0};
    uint16_t lastAckCommand{0};
    uint8_t lastAckResult{MAV_RESULT_FAILED};
    bool isInited{false};
    bool hasCommandAck{false};

    auto receiveLoop() -> void;
    auto handleIncomingMessage(const mavlink_message_t& msg) -> void;

    auto prepareCommandAckWait(uint16_t command) -> void;
    auto waitCommandAck(uint16_t command, std::chrono::milliseconds timeout) -> bool;

    void init();
};
