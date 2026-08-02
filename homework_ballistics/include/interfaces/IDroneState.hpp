#pragma once
#include <memory>

struct DroneContext;
struct DroneCommand;
class DronePhysics;

class IDroneState {
  public:
    [[deprecated]] virtual auto execute(DroneContext& context) -> std::unique_ptr<IDroneState> = 0;
    virtual auto threadExecute(DroneContext& context) -> DroneCommand = 0;
    virtual ~IDroneState() = default;
};