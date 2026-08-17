#include "mavlink/MavlinkClient.hpp"
#include "common.hpp"
#include "mavlink/HeartbeatWorker.hpp"
#include "mavlink/SysStatusWorker.hpp"
#include "mavlink/GlobalPositionWorker.hpp"
#include "mavlink/AttitudeWorker.hpp"
#include <arpa/inet.h>
#include <unistd.h>
#include <array>
#include <span>

MavlinkClient::MavlinkClient(const MavlinkConfig& mavlinkConfig, ThreadDronePhysics& dronePhysics)
    : mavlinkConfig(mavlinkConfig)
    , heartbeatWorker(std::make_unique<HeartbeatWorker>(this))
    , sysStatusWorker(std::make_unique<SysStatusWorker>(this))
    , globalPositionWorker(std::make_unique<GlobalPositionWorker>(mavlinkConfig, this, dronePhysics))
    , attitudeWorker(std::make_unique<AttitudeWorker>(this, dronePhysics))
{
}

auto MavlinkClient::init() -> void
{
    mavlinkTransport.resource = socket(AF_INET, SOCK_DGRAM, 0);

    if (mavlinkTransport.resource < 0) {
        LOG("Mavlink client is not inited: Failed to create UDP socket.");

        return;
    }

    // int reuseAddress = 1;

    // if (setsockopt(mavlinkTransport.resource, SOL_SOCKET, SO_REUSEADDR, &reuseAddress, sizeof(reuseAddress)) < 0) {
    //     LOG("Failed to set SO_REUSEADDR, errno: " << errno << " (" << strerror(errno) << ")");
    // }

    // sockaddr_in localAddress{};
    // localAddress.sin_family = AF_INET;
    // localAddress.sin_port = htons(LOCAL_PORT);
    // localAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    // if (bind(mavlinkTransport.resource, reinterpret_cast<sockaddr*>(&localAddress), sizeof(localAddress)) < 0) {
    //     LOG("Failed to bind MAVLink UDP socket on 0.0.0.0:" << LOCAL_PORT << ", errno: " << errno << " (" << strerror(errno) << ")");

    //     close(mavlinkTransport.resource);
    //     mavlinkTransport.resource = -1;

    //     return;
    // }

    // LOG("MAVLink UDP socket bound on 0.0.0.0:" << LOCAL_PORT);

    sockaddr_in qgcAddress{};
    qgcAddress.sin_family = AF_INET;
    qgcAddress.sin_port = htons(mavlinkConfig.qgsConfig.port);

    if (inet_pton(AF_INET, mavlinkConfig.qgsConfig.ip.c_str(), &qgcAddress.sin_addr) <= 0) {
        close(mavlinkTransport.resource);
        LOG("Mavlink client is not inited: Invalid QGC IP address.");

        return;
    }

    mavlinkTransport.addresss = qgcAddress;
    isInited = true;
};

auto MavlinkClient::getTransport() const -> MavlinkTransport
{
    return mavlinkTransport;
};

auto MavlinkClient::getSystemId() const -> int
{
    return mavlinkConfig.systemId;
};

auto MavlinkClient::sendMessage(const mavlink_message_t& message) -> void
{
    if (!isInited) {
        DEBUG("Failed to send message: client is not inited");

        return;
    }

    std::array<uint8_t, MAVLINK_MAX_PACKET_LEN> buffer{};
    const uint16_t packetLength = mavlink_msg_to_send_buffer(buffer.data(), &message);

    if (packetLength <= 0) {
        DEBUG("Failed to send message: wrong message");

        return;
    }

    const ssize_t sentBytes =
        sendto(mavlinkTransport.resource,
               buffer.data(),
               packetLength,
               0,
               reinterpret_cast<const sockaddr*>(&mavlinkTransport.addresss),  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
               sizeof(mavlinkTransport.addresss));

    if (sentBytes < 0) {
        DEBUG("Failed to send message");

        return;
    }

    DEBUG("Sent bytes: " << sentBytes);
}

auto MavlinkClient::prepareCommandAckWait(uint16_t command) -> void
{
    std::lock_guard<std::mutex> lock(commandAckMutex);

    expectedAckCommand = command;
    lastAckCommand = 0;
    lastAckResult = MAV_RESULT_FAILED;
    hasCommandAck = false;
}

auto MavlinkClient::waitCommandAck(uint16_t command, std::chrono::milliseconds timeout) -> bool
{
    std::unique_lock<std::mutex> lock(commandAckMutex);

    const bool received =
        commandAckCondition.wait_for(lock, timeout, [this, command]() { return hasCommandAck && lastAckCommand == command; });

    if (!received) {
        DEBUG("Timeout waiting COMMAND_ACK for command: " << command);

        return false;
    }

    if (lastAckResult != MAV_RESULT_ACCEPTED) {
        DEBUG("COMMAND_ACK received, but result is not ACCEPTED:" << " command=" << lastAckCommand
                                                                  << " result=" << static_cast<int>(lastAckResult));

        return false;
    }

    return true;
}

auto MavlinkClient::sendMessageWithAck(const mavlink_message_t& message) -> bool
{
    if (!isInited) {
        DEBUG("Failed to send message: client is not inited");

        return false;
    }

    constexpr auto ackTimeout = std::chrono::milliseconds(MavlinkClient::COMMAND_ACK_TIMEOUT_MS);

    prepareCommandAckWait(MAV_CMD_USER_1);

    for (int attempt = 1; attempt <= MavlinkClient::COMMAND_ACK_MAX_ATTEMPTS; ++attempt) {
        sendMessage(message);

        if (waitCommandAck(MAV_CMD_USER_1, ackTimeout)) {
            DEBUG("Drop command accepted. Stop retrying.");

            return true;
        }

        DEBUG("Drop command was not acknowledged. Retrying if attempts remain.");
    }

    return false;
}

auto MavlinkClient::receiveLoop() -> void
{
    std::array<uint8_t, 2048> recvBuffer{};
    mavlink_message_t incomingMessage{};
    mavlink_status_t mavStatus{};

    DEBUG("Mavlink RX Thread started.");

    while (isRunning) {  // stop request
        sockaddr_storage from{};
        socklen_t fromLen = sizeof(from);

        // This call blocks until QGC sends a packet back
        ssize_t bytesRead = recvfrom(mavlinkTransport.resource,
                                     recvBuffer.data(),
                                     recvBuffer.size(),
                                     0,
                                     reinterpret_cast<sockaddr*>(&from),  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
                                     &fromLen);

        if (bytesRead < 0) {
            // Socket was closed by stop() or an actual error occurred
            DEBUG("recvfrom failed with errno: " << errno << " (" << strerror(errno) << ")");

            continue;
        }

        auto bufferSpan = std::span<const uint8_t>(recvBuffer.data(), static_cast<size_t>(bytesRead));

        for (uint8_t byte : bufferSpan) {
            if (mavlink_parse_char(MAVLINK_COMM_0, byte, &incomingMessage, &mavStatus) != 0U) {
                handleIncomingMessage(incomingMessage);
            }
        }
    }

    DEBUG("Mavlink RX Thread stopped.");
}

auto MavlinkClient::handleIncomingMessage(const mavlink_message_t& msg) -> void
{
    switch (msg.msgid) {
        case MAVLINK_MSG_ID_PARAM_REQUEST_LIST: {
            DEBUG("QGC requested params. Sending dummy response.");
            mavlink_message_t response{};
            char param_id[16] = "SYS_STATUS";
            mavlink_msg_param_value_pack(getSystemId(), COMP_ID, &response, std::data(param_id), 1.0F, MAV_PARAM_TYPE_REAL32, 1, 0);
            sendMessage(response);

            break;
        }

        case MAVLINK_MSG_ID_MISSION_REQUEST_LIST: {
            DEBUG("QGC requested missions.");

            break;
        }

        case MAVLINK_MSG_ID_COMMAND_LONG: {
            mavlink_command_long_t cmd{};
            mavlink_msg_command_long_decode(&msg, &cmd);

            switch (cmd.command) {
                case MAV_CMD_REQUEST_MESSAGE: {
                    DEBUG("Command: MAV_CMD_REQUEST_MESSAGE, requested message id: " << cmd.param1);

                    if (static_cast<int>(cmd.param1) == MAVLINK_MSG_ID_AUTOPILOT_VERSION) {
                        mavlink_message_t ack{};
                        mavlink_msg_command_ack_pack(
                            getSystemId(), COMP_ID, &ack, MAV_CMD_REQUEST_MESSAGE, MAV_RESULT_ACCEPTED, 0, 0, msg.sysid, msg.compid);
                        sendMessage(ack);
                    }

                    break;
                }

                case MAV_CMD_IMAGE_START_CAPTURE: {
                    DEBUG("MAV_CMD_IMAGE_START_CAPTURE:" << " interval=" << cmd.param2 << " count=" << cmd.param3
                                                         << " sequence=" << cmd.param4);

                    mavlink_message_t ack{};
                    mavlink_msg_command_ack_pack(
                        getSystemId(), COMP_ID, &ack, MAV_CMD_IMAGE_START_CAPTURE, MAV_RESULT_ACCEPTED, 0, 0, msg.sysid, msg.compid);
                    sendMessage(ack);

                    break;
                }

                case MAV_CMD_CONTROL_HIGH_LATENCY: {
                    DEBUG("MAV_CMD_CONTROL_HIGH_LATENCY: requested=" << cmd.param1);

                    mavlink_message_t ack{};
                    mavlink_msg_command_ack_pack(
                        getSystemId(), COMP_ID, &ack, MAV_CMD_CONTROL_HIGH_LATENCY, MAV_RESULT_ACCEPTED, 0, 0, msg.sysid, msg.compid);

                    sendMessage(ack);
                    break;
                }

                case MAV_CMD_REQUEST_CAMERA_INFORMATION: {
                    DEBUG("QGC requested camera information");
                    mavlink_message_t ack{};
                    mavlink_msg_command_ack_pack(getSystemId(),
                                                 COMP_ID,
                                                 &ack,
                                                 MAV_CMD_REQUEST_CAMERA_INFORMATION,
                                                 MAV_RESULT_UNSUPPORTED,
                                                 0,
                                                 0,
                                                 msg.sysid,
                                                 msg.compid);
                    sendMessage(ack);

                    break;
                }

                default: {
                    DEBUG("Unhandled COMMAND_LONG command: " << cmd.command);
                    mavlink_message_t ack{};
                    mavlink_msg_command_ack_pack(
                        getSystemId(), COMP_ID, &ack, cmd.command, MAV_RESULT_UNSUPPORTED, 0, 0, msg.sysid, msg.compid);
                    sendMessage(ack);

                    break;
                }
            }

            break;
        }

        case MAVLINK_MSG_ID_COMMAND_ACK: {
            mavlink_command_ack_t cmd{};
            mavlink_msg_command_ack_decode(&msg, &cmd);

            DEBUG("Received COMMAND_ACK:" << " command=" << cmd.command << " result=" << static_cast<int>(cmd.result));
            lastAckCommand = cmd.command;
            lastAckResult = cmd.result;

            if (cmd.result == MAV_RESULT_ACCEPTED) {
                DEBUG("COMMAND_ACK accepted for command: " << cmd.command);
                hasCommandAck = true;
            }
            else {
                DEBUG("COMMAND_ACK not accepted:" << " command=" << cmd.command << " result=" << static_cast<int>(cmd.result));
            }

            break;
        }

        default:
            DEBUG("Unknown message id: " << msg.msgid);

            break;
    }
}

auto MavlinkClient::start() -> void
{
    init();

    if (!isInited) {
        return;
    }

    isRunning = true;
    receiveThread = std::thread(&MavlinkClient::receiveLoop, this);

    heartbeatWorker->start();
    sysStatusWorker->start();
    globalPositionWorker->start();
    attitudeWorker->start();
}

auto MavlinkClient::isThreadReady() const -> bool
{
    return heartbeatWorker->isThreadReady() && sysStatusWorker->isThreadReady() && globalPositionWorker->isThreadReady() &&
           attitudeWorker->isThreadReady() && isRunning;
}

auto MavlinkClient::wait() -> void
{
    heartbeatWorker->wait();
    sysStatusWorker->wait();
    globalPositionWorker->wait();
    attitudeWorker->wait();
}

auto MavlinkClient::stop() -> void
{
    heartbeatWorker->stop();
    sysStatusWorker->stop();
    globalPositionWorker->stop();
    attitudeWorker->stop();

    if (isRunning) {
        isRunning = false;

        if (mavlinkTransport.resource >= 0) {
            close(mavlinkTransport.resource);
            mavlinkTransport.resource = -1;
        }

        if (receiveThread.joinable()) {
            receiveThread.join();
        }
    }
}

MavlinkClient::~MavlinkClient()
{
    stop();
}
