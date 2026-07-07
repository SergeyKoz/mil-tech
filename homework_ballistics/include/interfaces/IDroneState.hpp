#pragma once
#include <memory>

struct DroneContext;

class IDroneState {
  public:
    virtual auto execute(DroneContext& context) -> std::unique_ptr<IDroneState> = 0;
    virtual ~IDroneState() = default;
};