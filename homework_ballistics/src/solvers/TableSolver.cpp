#include "solvers/TableSolver.hpp"
#include "solvers/BallisticTable.hpp"

BallisticTable TableSolver::table;

TableSolver::TableSolver(std::string ballicticTableFile)
    : ballicticTableFile(std::move(ballicticTableFile))
{
}

auto TableSolver::init() -> void
{
    table.load(ballicticTableFile);
}

auto TableSolver::calcDropParameters(const AmmoParams& ammo, float v0, float z0) -> DropParameters
{
    auto result = table.lookup(z0, v0, ammo.mass, ammo.drag, ammo.lift);

    return {
        .time = result.t,
        .distance = result.hDist,
    };
};
