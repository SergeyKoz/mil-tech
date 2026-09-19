#include "mavlink/SysStatusWorker.hpp"
#include "mavlink/MavlinkClient.hpp"

SysStatusWorker::SysStatusWorker(MavlinkClient* mavlinkClient)
    : IntervalWorker(std::chrono::milliseconds(1000), 1)
    , mavlinkClient(mavlinkClient)
{
}

auto SysStatusWorker::intervalTask() -> void
{
    mavlink_message_t message{};

    constexpr uint32_t sensors = MAV_SYS_STATUS_SENSOR_3D_GYRO | MAV_SYS_STATUS_SENSOR_3D_ACCEL | MAV_SYS_STATUS_SENSOR_3D_MAG |
                                 MAV_SYS_STATUS_SENSOR_ABSOLUTE_PRESSURE | MAV_SYS_STATUS_SENSOR_GPS |
                                 MAV_SYS_STATUS_SENSOR_ATTITUDE_STABILIZATION | MAV_SYS_STATUS_SENSOR_YAW_POSITION |
                                 MAV_SYS_STATUS_SENSOR_XY_POSITION_CONTROL | MAV_SYS_STATUS_SENSOR_Z_ALTITUDE_CONTROL;

    constexpr uint16_t load = 300;              // 30.0%
    constexpr uint16_t voltageBattery = 12000;  // mV
    constexpr int16_t currentBattery = -1;      // cA, -1 = unknown
    constexpr int8_t batteryRemaining = 80;     // %
    constexpr uint16_t dropRateComm = 0;        // 0.01%
    constexpr uint16_t errorsComm = 0;
    constexpr uint16_t errorsCount1 = 0;
    constexpr uint16_t errorsCount2 = 0;
    constexpr uint16_t errorsCount3 = 0;
    constexpr uint16_t errorsCount4 = 0;
    constexpr uint32_t sensorsExtended = 0;

    mavlink_msg_sys_status_pack(mavlinkClient->getSystemId(),
                                MavlinkClient::COMP_ID,
                                &message,
                                sensors,
                                sensors,
                                sensors,
                                load,
                                voltageBattery,
                                currentBattery,
                                batteryRemaining,
                                dropRateComm,
                                errorsComm,
                                errorsCount1,
                                errorsCount2,
                                errorsCount3,
                                errorsCount4,
                                sensorsExtended,
                                sensorsExtended,
                                sensorsExtended);

    mavlinkClient->sendMessage(message);
}

SysStatusWorker::~SysStatusWorker() = default;