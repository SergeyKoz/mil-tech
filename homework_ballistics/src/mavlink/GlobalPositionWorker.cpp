#include "common.hpp"
#include "mavlink/GlobalPositionWorker.hpp"
#include "mavlink/MavlinkClient.hpp"
#include "ThreadDronePhysics.hpp"

GlobalPositionWorker::GlobalPositionWorker(MavlinkConfig mavlinkConfig, MavlinkClient* mavlinkClient, ThreadDronePhysics& dronePhysics)
    : IntervalWorker(std::chrono::milliseconds(200), 1)
    , mavlinkConfig(std::move(mavlinkConfig))
    , mavlinkClient(mavlinkClient)
    , dronePhysics(&dronePhysics)
{
}

auto GlobalPositionWorker::intervalTask() -> void
{
    auto originLocation = mavlinkConfig.qgsConfig.originLocation;
    auto originLatitudeDeg = originLocation.latitude;
    auto originLongitudeDeg = originLocation.longtitude;

    constexpr double earthRadiusMeters = 6378137.0;

    const auto telemetry = dronePhysics->getTelemetry();

    auto altitudeMm = static_cast<int32_t>(telemetry.altitude * 1000);
    auto relativeAltitudeMm = altitudeMm;

    const double latitudeOffsetDeg = telemetry.position.y / earthRadiusMeters * 180.0 / M_PI;
    const double longitudeOffsetDeg =
        telemetry.position.x / (earthRadiusMeters * std::cos(originLatitudeDeg * M_PI / 180.0)) * 180.0 / M_PI;

    const auto latitude = static_cast<int32_t>((originLatitudeDeg + latitudeOffsetDeg) * 1e7);
    const auto longitude = static_cast<int32_t>((originLongitudeDeg + longitudeOffsetDeg) * 1e7);

    const auto vx = static_cast<int16_t>(telemetry.speed.x * 100.0F);
    const auto vy = static_cast<int16_t>(telemetry.speed.y * 100.0F);
    constexpr int16_t vz = 0;

    const auto headingDeg = telemetry.direction * 180.0F / std::numbers::pi_v<float>;
    const auto heading = static_cast<uint16_t>(std::fmod(headingDeg * 100.0F + 36000.0F, 36000.0F));

    mavlink_message_t message{};

    mavlink_msg_global_position_int_pack(mavlinkClient->getSystemId(),
                                         MavlinkClient::COMP_ID,
                                         &message,
                                         static_cast<uint32_t>(telemetry.timeSinceStart * 1000.0F),
                                         latitude,
                                         longitude,
                                         altitudeMm,
                                         relativeAltitudeMm,
                                         vx,
                                         vy,
                                         vz,
                                         heading);

    LOG("Global_position: latitude: " << latitude << " longitude: " << longitude);

    mavlinkClient->sendMessage(message);
}

GlobalPositionWorker::~GlobalPositionWorker() = default;