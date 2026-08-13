#pragma once

#include <string>

class I2CBus {
  public:
    I2CBus(std::string resource);

    auto open() -> void;
    auto getBus() const -> int;

    ~I2CBus();

  private:
    int bus = 0;
    std::string resource;
};
