#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <thread>

class UdpSocket;

struct Waypoint {
    float north = 0.0f;
    float east = 0.0f;

    bool operator==(const Waypoint &other) const
    {
        const float epsilon = 1e-5f;

        return std::abs(north - other.north) < epsilon && std::abs(east - other.east) < epsilon;
    }
};

class AutoStub {
public:
    AutoStub(uint16_t listenPort);
    ~AutoStub();

    Waypoint getWaypoint();
    bool hasWaypoint();
private:
    mutable std::mutex waypointMutex;
    Waypoint waypoint;
    std::atomic<bool> running;
    std::unique_ptr<UdpSocket> socket;
    std::thread readerThread;

    void readLoop();
    bool parseWaypoint(const std::string& message, Waypoint& parsed) const;
    bool waypointReceived = false;
};
