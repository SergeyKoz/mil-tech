#include "mavlink/AttitudeWorker.hpp"
#include "mavlink/MavlinkClient.hpp"
#include "ThreadDronePhysics.hpp"

AttitudeWorker::AttitudeWorker(MavlinkClient* mavlinkClient, ThreadDronePhysics& dronePhysics)
    : IntervalWorker(std::chrono::milliseconds(500), 1)
    , mavlinkClient(mavlinkClient)
    , dronePhysics(&dronePhysics)
{
}

auto AttitudeWorker::intervalTask() -> void
{
    const auto telemetry = dronePhysics->getTelemetry();

    constexpr float roll = 0.0F;
    constexpr float pitch = 0.0F;

    float yaw = std::fmod(telemetry.direction, 2.0F * std::numbers::pi_v<float>);

    if (yaw < 0.0F) {
        yaw += 2.0F * std::numbers::pi_v<float>;
    }

    constexpr float rollSpeed = 0.0F;
    constexpr float pitchSpeed = 0.0F;
    constexpr float yawSpeed = 0.0F;

    mavlink_message_t message{};

    mavlink_msg_attitude_pack(mavlinkClient->getSystemId(),
                              MavlinkClient::COMP_ID,
                              &message,
                              static_cast<uint32_t>(telemetry.timeSinceStart * 1000.0F),
                              roll,
                              pitch,
                              yaw,
                              rollSpeed,
                              pitchSpeed,
                              yawSpeed);

    mavlinkClient->sendMessage(message);
}

AttitudeWorker::~AttitudeWorker() = default;