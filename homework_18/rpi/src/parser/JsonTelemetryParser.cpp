#include "parser/JsonTelemetryParser.hpp"
#include "SensorController.hpp"
#include "common.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

JsonTelemetryParser::JsonTelemetryParser(const std::shared_ptr<SensorController> &sensorController) : sensorController(sensorController)
{
}

auto JsonTelemetryParser::parse(const std::string &telemetry) -> void
{

    try
    {
        auto j = json::parse(telemetry);
        auto device = j["device"].get<std::string>();

        if (device == "mpu5060")
        {
            MPU5060Data data;
            data.ax = j["data"]["ax"].get<float>();
            data.ay = j["data"]["ay"].get<float>();
            data.az = j["data"]["az"].get<float>();
            data.gx = j["data"]["gx"].get<float>();
            data.gy = j["data"]["gy"].get<float>();
            data.gz = j["data"]["gz"].get<float>();
            sensorController->updateMPU5060Data(data);
            LOG(data.toString());
        }
        else if (device == "bmp280")
        {
            BMP280Data data;
            data.temperature = j["data"]["temperature"].get<float>();
            data.pressure = j["data"]["pressure"].get<float>();
            data.altitude = j["data"]["altitude"].get<float>();
            sensorController->updateBMP280Data(data);
            LOG(data.toString());
        }
        else if (device == "htu21")
        {
            HTU21Data data;
            data.temperature = j["data"]["temperature"].get<float>();
            data.humidity = j["data"]["humidity"].get<float>();
            sensorController->updateHTU21Data(data);
            LOG(data.toString());
        }
        else
        {
            LOG("Unknown device: " + device);
        }
    }
    catch (const nlohmann::json::parse_error &e)
    {
        LOG("Failed to parse sensor data");
    }
}
