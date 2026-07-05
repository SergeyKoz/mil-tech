#pragma once

#include <string>
#include <vector>
#include "drone_link.h"

class IUartListener;

class RpiCheckerUART {
  public:
    RpiCheckerUART(std::string uartPort);
    auto init() -> void;
    auto addListener(IUartListener &listener) -> void;
    auto listenPackages() -> void;
    auto writeControl(const dlink::Control &control) -> void;
    ~RpiCheckerUART();

  private:
    int uart;
    std::string uartPort;
    std::vector<IUartListener *> listeners;

    auto openUart(const char *dev) -> int;
    auto processPacket(uint8_t type, const uint8_t *payload, uint8_t len) -> void;
};
