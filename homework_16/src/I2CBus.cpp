#include "I2CBus.hpp"
#include <iostream>
#include <fcntl.h>
#include <unistd.h>

I2CBus::I2CBus(std::string resource)
    : bus(-1)
    , resource(std::move(resource))
{
}

auto I2CBus::open() -> void
{
    bus = ::open(resource.c_str(), O_RDWR);

    if (bus < 0) {
        throw std::invalid_argument("Can't open I2C bus");
    }
}

auto I2CBus::getBus() const -> int
{
    return bus;
};

I2CBus::~I2CBus()
{
    ::close(bus);
}