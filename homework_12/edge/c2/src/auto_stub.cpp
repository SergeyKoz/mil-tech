#include "auto_stub.hpp"
#include "udp_socket.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <chrono>
#include <vector>

AutoStub::AutoStub(uint16_t listenPort):
    waypoint({})
    , running(true)
    , socket(std::make_unique<UdpSocket>(listenPort))
    , readerThread(&AutoStub::readLoop, this)
{
    std::cout << "[AutoStub] listening on UDP port " << listenPort << "\n";
}

Waypoint AutoStub::getWaypoint() {
    std::lock_guard<std::mutex> lock(waypointMutex);

    return waypoint;
}

bool AutoStub::hasWaypoint() {
    std::lock_guard<std::mutex> lock(waypointMutex);

    if (waypointReceived) {
        waypointReceived = false;

        return true;
    }

    return false;
}

AutoStub::~AutoStub() {
    running = false;

    if (readerThread.joinable()) {
        readerThread.join();
    }
}

void AutoStub::readLoop() {
    std::cout << "[AutoStub] Start loop " << "\n";
    std::vector<char> buffer(1024);
    sockaddr_in senderAddr{};

    auto delay = std::chrono::milliseconds(100);

    while (running) {
        const ssize_t bytesReceived = socket->recv(buffer.data(), buffer.size(), senderAddr);

        if (bytesReceived > 0) {
            char ipStr[INET_ADDRSTRLEN] = {};
            inet_ntop(AF_INET, &(senderAddr.sin_addr), ipStr, INET_ADDRSTRLEN);
            const uint16_t senderPort = ntohs(senderAddr.sin_port);
            const std::string message(buffer.data(), bytesReceived);

            std::cout << "[AutoStub] received "
                      << bytesReceived
                      << " bytes from "
                      << ipStr
                      << ":"
                      << senderPort
                      << " -> "
                      << message
                      << "\n";

            Waypoint parsed{};
            if (parseWaypoint(message, parsed)) {
                if (parsed != waypoint) {
                    std::lock_guard<std::mutex> lock(waypointMutex);
                    waypoint = parsed;
                    waypointReceived = true;
                }

                if (waypointReceived) {
                    std::cout << "[AutoStub] waypoint updated: north="
                          << parsed.north
                          << ", east="
                          << parsed.east
                          << "\n";
                }
            } else {
                std::cerr << "[AutoStub] ignored message: cannot parse waypoint\n";
            }
        } else if (bytesReceived == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::this_thread::sleep_for(delay); // std::chrono::milliseconds(100)
            } else {
                std::cerr << "[AutoStub] UDP receive error\n";
                std::this_thread::sleep_for(delay);
            }
        } else {
            std::this_thread::sleep_for(delay);
        }
    }
}

bool AutoStub::parseWaypoint(const std::string& message, Waypoint& parsed) const {
    try {
        const auto json = nlohmann::json::parse(message);

        if (json.contains("north_m") && json.contains("east_m")) {
            parsed.north = json.at("north_m").get<float>();
            parsed.east = json.at("east_m").get<float>();
            return true;
        }

        return false;
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "[AutoStub] invalid JSON: " << e.what() << "\n";

        return false;
    }
}
