#pragma once

#include <string>
#include <memory>
#include "interfaces/IParser.hpp"

class SensorController;

class JsonTelemetryParser : public IParser
{
public:
  JsonTelemetryParser(const std::shared_ptr<SensorController> &sensorController);
  auto parse(const std::string &telemetry) -> void;

private:
  std::shared_ptr<SensorController> sensorController;
};
