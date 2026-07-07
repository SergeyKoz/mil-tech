#pragma once

#include "interfaces/IBallisticsSolver.hpp"

struct BallisticTable;

class TableSolver : public IBallisticsSolver {
  public:
    TableSolver(std::string ballicticTableFile);
    auto init() -> void override;
    auto calcDropParameters(const AmmoParams& ammo, float v0, float z0) -> DropParameters override;

  private:
    static BallisticTable table;
    std::string ballicticTableFile;
};