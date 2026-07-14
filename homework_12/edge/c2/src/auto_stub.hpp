#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <thread>

class UdpSocket;

struct Waypoint {
    float north = 0.0f;
    float east = 0.0f;
};

class AutoStub {
public:
    AutoStub(uint16_t listenPort);
    ~AutoStub();

    Waypoint getWaypoint();
    bool hasWaypoint() const;
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
