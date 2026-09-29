#include "ballistics_simulator/providers/targets_provider.hpp"
#include <memory>
#include <vector>

namespace ballistics_simulator
{
    auto TargetsProvider::isReady() -> bool
    {
        return targetsCount != 0 && static_cast<int>(currentTargets.size()) == targetsCount && static_cast<int>(previousTargets.size()) == targetsCount;
    }

    auto TargetsProvider::getTarget(int index) -> TargetTelemetry
    {
        return currentTargets.at(index);
    }

    auto TargetsProvider::getTargetsCount() -> int
    {
        return targetsCount;
    }

    auto TargetsProvider::setTargetsCount(int count) -> void
    {
        targetsCount = count;
    }

    auto TargetsProvider::setTarget(int index, Coord pos, float time) -> void
    {
        if (!currentTargets.contains(index))
        {
            TargetTelemetry curentTarget = {.position = pos, .speed = {}};
            currentTargetsTimes.insert_or_assign(index, time);
            currentTargets.insert_or_assign(index, curentTarget);
        }
        else
        {
            auto currentTime = currentTargetsTimes.at(index);
            auto currentTarget = currentTargets.at(index);

            if (std::abs(currentTime - time) > epsilon)
            {
                auto previousTarget = !previousTargets.contains(index) ? TargetTelemetry{} : previousTargets.at(index);
                previousTarget = {.position = currentTarget.position, .speed = previousTarget.speed};
                previousTargetsTimes.insert_or_assign(index, currentTime);
                previousTargets.insert_or_assign(index, TargetTelemetry{.position = currentTarget.position, .speed = previousTarget.speed});
            }

            currentTargetsTimes.insert_or_assign(index, time);
        }

        if (currentTargets.contains(index) && previousTargets.contains(index))
        {
            auto dt = currentTargetsTimes.at(index) - previousTargetsTimes.at(index);
            auto prevPos = previousTargets.at(index).position;
            currentTargets.at(index) = {.position = pos, .speed = {.x = (pos.x - prevPos.x) / dt, .y = (pos.y - prevPos.y) / dt}};
        }
    }
} // namespace ballistics_simulator
