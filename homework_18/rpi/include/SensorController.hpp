#pragma once

#include "IntervalWorker.hpp"
#include "common.hpp"

class GpioController;

class SensorController : public IntervalWorker
{
public:
  explicit SensorController(std::chrono::milliseconds interval, std::unique_ptr<GpioController> gpio, float temperatureLevel);
  auto init() -> void;
  auto updateMPU5060Data(const MPU5060Data &sensorData) -> void;
  auto updateHTU21Data(const HTU21Data &sensorData) -> void;
  auto updateBMP280Data(const BMP280Data &sensorData) -> void;

  ~SensorController();

private:
  std::mutex dataMutex;
  std::unique_ptr<GpioController> gpio;
  float temperatureLevel;
  std::atomic<bool> isTemperatureHigh{false};

  MPU5060Data mpu5060data;
  HTU21Data htu21Data;
  BMP280Data bmp280Data;

  auto intervalTask() -> void override;
};
