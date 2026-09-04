#include "mavlink/MavlinkTargetProcessor.hpp"
#include "mavlink/MavlinkClient.hpp"
#include "common.hpp"

MavlinkTargetProcessor::MavlinkTargetProcessor(MavlinkConfig mavlinkConfig, MavlinkClient* mavlinkClient)
    : mavlinkConfig(std::move(mavlinkConfig))
    , mavlinkClient(mavlinkClient)
{
}

auto MavlinkTargetProcessor::processTarget(const SelectedTarget& target, const DroneTelemetry& droneTelemetry) -> void
{
    auto originLocation = mavlinkConfig.qgsConfig.originLocation;
    auto originLatitudeDeg = originLocation.latitude;
    auto originLongitudeDeg = originLocation.longtitude;

    constexpr double earthRadiusMeters = 6378137.0;

    const double latitudeOffsetDeg = target.telemetry.position.y / earthRadiusMeters * 180.0 / M_PI;
    const double longitudeOffsetDeg =
        target.telemetry.position.x / (earthRadiusMeters * std::cos(originLatitudeDeg * M_PI / 180.0)) * 180.0 / M_PI;

    const auto latitude = static_cast<float>(originLatitudeDeg + latitudeOffsetDeg);
    const auto longitude = static_cast<float>(originLongitudeDeg + longitudeOffsetDeg);
    const auto altitude = droneTelemetry.altitude;

    mavlink_message_t message{};

    constexpr uint8_t targetSystem = 0;
    constexpr uint8_t targetComponent = 0;
    constexpr uint8_t confirmation = 0;

    mavlink_msg_command_long_pack(mavlinkClient->getSystemId(),
                                  MavlinkClient::COMP_ID,
                                  &message,
                                  targetSystem,
                                  targetComponent,
                                  MAV_CMD_USER_1,
                                  confirmation,
                                  0.0F,
                                  0.0F,
                                  0.0F,
                                  0.0F,
                                  latitude,
                                  longitude,
                                  altitude);

    DEBUG("Sending MAV_CMD_USER_1 drop command:" << " lat=" << latitude << " lon=" << longitude << " alt=" << altitude);

    mavlinkClient->sendMessageWithAck(message);
    mavlinkClient->stop();
}
