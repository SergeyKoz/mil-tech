#include "c2_controller.hpp"
#include <log.hpp>
#include <c2_config.hpp>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>

int main() {
    auto log = std::make_shared<Log>("/var/log/c2/c2.log");
    constexpr auto tick_interval = std::chrono::milliseconds(1000);

    try {
        auto config = C2Config::load("/etc/c2/c2_config.json");
        log->info("config: fc_port=" + std::to_string(config.fcPort) + " as_port=" + std::to_string(config.asPort));

        C2Controller controller(config.fcPort, config.asPort, log);

        while (true) {
            controller.tick();
            std::this_thread::sleep_for(tick_interval);
        }
    } catch (const std::invalid_argument &ex) {
        std::cerr << ex.what() << '\n';
        log->error(ex.what());

        return 1;
    } catch (const std::exception &ex) {
        std::cerr << ex.what() << '\n';
        log->error(ex.what());

        return 1;
    }

    return 0;
}
