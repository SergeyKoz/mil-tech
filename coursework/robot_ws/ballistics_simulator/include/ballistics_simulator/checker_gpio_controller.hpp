#pragma once

#include <string>
#include <vector>
#include "drone_link.h"
#include <thread>
#include <atomic>

// class IUartListener;

struct gpiod_line_request;
struct gpiod_chip;

namespace ballistics_simulator
{

  class CheckerGPIOController
  {
  public:
    CheckerGPIOController(std::string chipName, uint startLine, uint dropLine);
    auto init() -> void;
    auto start() -> void;
    auto drop() -> void;
    ~CheckerGPIOController();

  private:
    std::string chipName;
    uint startLine;
    uint dropLine;

    gpiod_line_request *request;
    gpiod_chip *chip;
  };

} // namespace ballistics_simulator