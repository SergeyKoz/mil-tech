#pragma once

struct DroneContext;
struct DroneCommand;

class IDroneState
{
public:
  virtual auto threadExecute(DroneContext &context) -> DroneCommand = 0;
  virtual ~IDroneState() = default;
};