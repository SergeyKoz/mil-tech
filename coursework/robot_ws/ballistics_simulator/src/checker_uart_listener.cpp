#include "ballistics_simulator/checker_uart_listener.hpp"
// #include "common.hpp"
#include "interfaces/uart_listener_interface.hpp"
#include <iostream>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

namespace ballistics_simulator {

CheckerUARTListener::CheckerUARTListener(std::string uartPort)
    : uart(0)
    , uartPort(std::move(uartPort))
{
}

auto CheckerUARTListener::init() -> void
{
    uart = openUart(uartPort.c_str());
}

auto CheckerUARTListener::start() -> void
{
    if (isRunning.load()) {
        return;
    }

    isRunning.store(true);
    listenerThread = std::thread(&CheckerUARTListener::listenPackages, this);
}

auto CheckerUARTListener::stop() -> void
{
    if (!isRunning.load()) {
        return;
    }

    isRunning.store(false);

    if (listenerThread.joinable()) {
        listenerThread.join();
    }
}

auto CheckerUARTListener::addListener(IUartListener &listener) -> void
{
    listeners.push_back(&listener);
}

auto CheckerUARTListener::listenPackages() -> void
{
    dlink::Parser parser;
    uint8_t byte = 0;
    uint8_t type = 0;
    uint8_t payload[256];
    uint8_t len = 0;

    while (isRunning.load()) {
        ssize_t n = read(uart, &byte, 1);

        if (n <= 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(500));

            continue;
        }

        if (parser.feed(byte, type, &payload[0], len)) {
            processPacket(type, &payload[0], len);
        }
    }
}

auto CheckerUARTListener::writeControl(const dlink::Control &control) const -> void
{
    uint8_t out[64];
    size_t m = dlink::encode(dlink::PKT_CONTROL, &control, sizeof control, &out[0]);
    write(uart, &out, m);
}

CheckerUARTListener::~CheckerUARTListener()
{
    stop();

    if (uart >= 0) {
        close(uart);
        uart = -1;
    }
}

auto CheckerUARTListener::openUart(const char *dev) -> int
{
    int fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK);  // NOLINT(cppcoreguidelines-pro-type-vararg)

    if (fd < 0) {
        perror("open");
        return -1;
    }

    termios tio{};
    tcgetattr(fd, &tio);
    cfmakeraw(&tio);  // 8N1, без обробки символів
    cfsetispeed(&tio, B115200);
    cfsetospeed(&tio, B115200);  // швидкість з обох боків однакова!
    tio.c_cflag |= (CLOCAL | CREAD);
    tcsetattr(fd, TCSANOW, &tio);

    return fd;
}

void CheckerUARTListener::processPacket(uint8_t type, const uint8_t *payload, uint8_t len)
{
    switch (type) {
        case dlink::PKT_TELEMETRY: {
            if (len != sizeof(dlink::Telemetry)) {
                return;
            }

            dlink::Telemetry t{};
            std::memcpy(&t, payload, sizeof(t));

            for (IUartListener *listener : listeners) {
                if (listener != nullptr) {
                    listener->updateTelemetry(t);
                }
            }

            break;
        }

        case dlink::PKT_TARGET: {
            if (len != sizeof(dlink::TargetPos)) {
                return;
            }

            dlink::TargetPos target{};
            std::memcpy(&target, payload, sizeof(target));

            for (IUartListener *listener : listeners) {
                if (listener != nullptr) {
                    listener->updateTargetPosition(target);
                }
            }

            break;
        }

        case dlink::PKT_AMMO: {
            if (len != sizeof(dlink::AmmoCfg)) {
                return;
            }

            dlink::AmmoCfg ammo{};
            std::memcpy(&ammo, payload, sizeof(ammo));

            for (IUartListener *listener : listeners) {
                if (listener != nullptr) {
                    listener->updateAmmoConfig(ammo);
                }
            }

            break;
        }

        case dlink::PKT_CONFIG: {
            if (len != sizeof(dlink::DroneCfg)) {
                return;
            }

            dlink::DroneCfg cfg{};
            std::memcpy(&cfg, payload, sizeof(cfg));

            for (IUartListener *listener : listeners) {
                if (listener != nullptr) {
                    listener->updateDroneConfig(cfg);
                }
            }

            break;
        }

        case dlink::PKT_CONTROL: {
            if (len != sizeof(dlink::Control)) {
                return;
            }

            dlink::Control control{};
            std::memcpy(&control, payload, sizeof(control));

            for (IUartListener *listener : listeners) {
                if (listener != nullptr) {
                    listener->updateControl(control);
                }
            }

            break;
        }
        case dlink::PKT_RESULT: {
            if (len != sizeof(dlink::Result)) {
                return;
            }

            dlink::Result result{};
            std::memcpy(&result, payload, sizeof(result));

            for (IUartListener *listener : listeners) {
                if (listener != nullptr) {
                    listener->updateResult(result);
                }
            }

            break;
        }
        default:
            return;
            // DEBUG("Unknown packet type: " << (int)type);
    }
}

}  // namespace ballistics_simulator