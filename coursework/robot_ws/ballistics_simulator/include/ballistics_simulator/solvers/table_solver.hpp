#pragma once

#include "interfaces/ballistics_solver_interface.hpp"

namespace ballistics_simulator
{
  struct BallisticTable;

  class TableSolver : public IBallisticsSolver
  {
  public:
    TableSolver(std::string ballicticTableFile);
    auto init() -> void override;
    auto calcDropParameters(const ballistics_simulator::AmmoParams &ammo, float v0, float z0) -> ballistics_simulator::DropParameters override;

  private:
    static BallisticTable table;
    std::string ballicticTableFile;
  };
}