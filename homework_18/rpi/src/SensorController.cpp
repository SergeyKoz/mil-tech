#include "SensorController.hpp"
#include "GpioController.hpp"
#include <iostream>

SensorController::SensorController(std::chrono::milliseconds interval, std::unique_ptr<GpioController> gpio, float temperatureLevel)
    : IntervalWorker(interval), gpio(std::move(gpio)), temperatureLevel(temperatureLevel)
{
}

auto SensorController::init() -> void
{
    gpio->init();
}

auto SensorController::updateMPU5060Data(const MPU5060Data &sensorData) -> void
{
    std::lock_guard<std::mutex> lock(dataMutex);
    mpu5060data = sensorData;
}

auto SensorController::updateHTU21Data(const HTU21Data &sensorData) -> void
{
    std::lock_guard<std::mutex> lock(dataMutex);
    htu21Data = sensorData;
}

auto SensorController::updateBMP280Data(const BMP280Data &sensorData) -> void
{
    std::lock_guard<std::mutex> lock(dataMutex);
    bmp280Data = sensorData;
}

auto SensorController::intervalTask() -> void
{
    auto isTempHigh = htu21Data.temperature > temperatureLevel;

    if (isTempHigh != isTemperatureHigh.load())
    {
        gpio->setTemperatureControlLine(isTempHigh);
        isTemperatureHigh = isTempHigh;
    }

    LOG("Temperature control line set to: " + std::string(isTempHigh ? "HIGH" : "LOW"));
}

SensorController::SensorController::~SensorController() = default;