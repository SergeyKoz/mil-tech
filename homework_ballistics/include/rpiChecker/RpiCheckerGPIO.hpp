#pragma once

#include <string>

struct gpiod_line_request;
struct gpiod_chip;

class RpiCheckerGPIO {
  public:
    RpiCheckerGPIO(std::string chipName, uint startLine, uint dropLine);
    auto init() -> void;
    auto start() -> void;
    auto drop() -> void;
    ~RpiCheckerGPIO();

  private:
    std::string chipName;
    uint startLine;
    uint dropLine;

    gpiod_line_request *request;
    gpiod_chip *chip;
};
