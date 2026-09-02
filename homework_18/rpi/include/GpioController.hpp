#pragma once

#include <string>

struct gpiod_line_request;
struct gpiod_chip;

class GpioController
{
public:
  GpioController(std::string chipName, uint controlLine);

  auto init() -> void;
  auto setTemperatureControlLine(bool isTemperatureHigh) -> void;

  ~GpioController();

private:
  std::string chipName;
  uint controlLine;
  gpiod_line_request *request;
  gpiod_chip *chip;
};
