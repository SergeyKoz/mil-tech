#include "c2_controller.hpp"
#include "auto_stub.hpp"
#include "fc_link.hpp"     // MAVSDK обгортка, API описано у fc_link.hpp
#include "udp_socket.hpp"  // UDP прийом, API описано у udp_socket.hpp
#include "log.hpp"
#include <string>

// static constexpr uint16_t STUB_PORT = 14560;
static constexpr const char* C2_HEALTH_PATH = "/tmp/c2_healthy";

struct C2Controller::Impl {
    Impl(uint16_t fc_port, std::shared_ptr<Log> log) : fc(fc_port), holdSent(false), log(log)
    {
    }

    C2State state = C2State::DISARMED;
    FcLink fc;
    bool holdSent;
    std::shared_ptr<Log> log;

    // TODO: додати FcLink, UdpSocket, лог-файл та прапорцi стану.
    // FcLink потребує fc_port у конструкторi Impl.
    // UdpSocket має слухати STUB_PORT.

    void transition(C2State next) {
        // TODO: якщо next != state, записати "PREV -> NEW" у stdout i лог,
        // потiм оновити state. Якщо стан не змiнився, нiчого не писати.

        if (next == state) {
            return;
        }

        log->info("state: " + stateToString(state) + " -> " + stateToString(next));
        holdSent = false;

        state = next;
    }

    const std::string stateToString(C2State state) {
        switch (state) {
            case C2State::DISARMED:
                return "DISARMED";
            case C2State::ARMED_HOLD:
                return "ARMED_HOLD";
            case C2State::ARMED_GUIDED:
                return "ARMED_GUIDED";
            case C2State::ARMED_MANUAL:
                return "ARMED_MANUAL";
        }

        return "UNKNOWN";
    }
};

C2Controller::C2Controller(uint16_t fc_port, uint16_t as_port, std::shared_ptr<Log> log)
    : impl_(std::make_unique<Impl>(fc_port, log))
    , autoStub(std::make_unique<AutoStub>(as_port))
    , log(log)
{
    // TODO: передати fc_port в Impl та вiдкрити /var/log/c2/c2.log.
//     (void)fc_port;
}

C2Controller::~C2Controller() = default;

void C2Controller::tick() {

    if (!healthcheck()) {
        return;
    }

    // TODO: healthcheck, оновлення C2State, читання точки маршруту,
    // передавання або блокування команди згiдно з поточним станом.
    auto fcState = readFcState();
    impl_->transition(fcState);
    handle(currentState());
}

C2State C2Controller::currentState() const {
    return impl_->state;
}

C2State C2Controller::readFcState() const {
    auto isArmed = impl_->fc.is_armed();

    if (!isArmed) {
        return C2State::DISARMED;
    }

    auto flightMode = impl_->fc.flight_mode();

    switch (flightMode) {
        case FcLink::FlightMode::Guided:
            return C2State::ARMED_GUIDED;
        case FcLink::FlightMode::Manual:
            return C2State::ARMED_MANUAL;
        case FcLink::FlightMode::Hold:
        case FcLink::FlightMode::Unknown:
            return C2State::ARMED_HOLD;
    }

    return C2State::ARMED_HOLD;
}

void C2Controller::handle(C2State state) const {
    switch (state) {
        case C2State::DISARMED:
            log->info("blocked: waypoint in DISARMED");

            break;

        case C2State::ARMED_HOLD:
            log->info("blocked: waypoint in ARMED_HOLD");

            if (!impl_->holdSent) {
                impl_->fc.hold();
                impl_->holdSent = true;
                log->info("sent HOLD to FC");
            }

            break;

            case C2State::ARMED_GUIDED:
                if (autoStub->hasWaypoint()) {
                    auto waypoint = autoStub->getWaypoint();
                    log->info("fwd: north=" + std::to_string(waypoint.north) + " east=" + std::to_string(waypoint.east));
                    impl_->fc.go_to_ned(waypoint.north, waypoint.east);
                }

                break;

            case C2State::ARMED_MANUAL:
                log->info("blocked: waypoint in ARMED_MANUAL");

                break;
    }
}

bool C2Controller::healthcheck() const {
    if (!impl_->fc.is_connected()) {
        std::remove(C2_HEALTH_PATH);
        log->info("FC Disconnected");

        return false;
    }

    std::ofstream healthy(C2_HEALTH_PATH);
    healthy << "ok\n";

    return true;
}
