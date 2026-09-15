#pragma once

#include <string>
#include <vector>
#include "drone_link.h"
#include <thread>
#include <atomic>

class IUartListener;

namespace ballistics_simulator {

class CheckerUARTListener {
  public:
    CheckerUARTListener(std::string uartPort);
    auto init() -> void;
    auto start() -> void;
    auto stop() -> void;
    auto addListener(IUartListener &listener) -> void;
    auto writeControl(const dlink::Control &control) const -> void;
    ~CheckerUARTListener();

  private:
    int uart{-1};
    std::string uartPort;
    std::vector<IUartListener *> listeners;

    std::atomic<bool> isRunning{false};
    std::thread listenerThread;

    static auto openUart(const char *dev) -> int;
    auto listenPackages() -> void;
    auto processPacket(uint8_t type, const uint8_t *payload, uint8_t len) -> void;
};

}  // namespace ballistics_simulator