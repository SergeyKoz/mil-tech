#include "providers/CheckerTargetProvider.hpp"
#include "common.hpp"
#include <memory>
#include <vector>
#include "Target.hpp"

void CheckerTargetProvider::load() {}

auto CheckerTargetProvider::isReady() -> bool
{
    return static_cast<int>(currentTargets.size()) == targetsCount && static_cast<int>(previousTargets.size()) == targetsCount;
}

auto CheckerTargetProvider::getTarget(int index) -> Target *
{
    return currentTargets.at(index).get();
}

auto CheckerTargetProvider::getTargetsCount() -> int
{
    return targetsCount;
}

auto CheckerTargetProvider::setTargetsCount(int count) -> void
{
    targetsCount = count;
}

auto CheckerTargetProvider::getTimeSteps() -> int
{
    return 0;
}

auto CheckerTargetProvider::setTarget(int index, Coord pos, float time) -> void
{
    if (!currentTargets.contains(index)) {
        auto curentTtarget = std::make_unique<Target>(std::vector<Coord>{pos}, 0);
        currentTargetsTimes.insert_or_assign(index, time);
        currentTargets.insert_or_assign(index, std::move(curentTtarget));
    }
    else {
        auto currentTime = currentTargetsTimes.at(index);
        auto *currentTarget = currentTargets.at(index).get();
        auto currentTtargetTelemetry = currentTarget->getTelemetry();

        if (std::abs(currentTime - time) > epsilon) {
            auto previousTtarget = !previousTargets.contains(index) ? std::make_unique<Target>(std::vector<Coord>{Coord{}}, 0)
                                                                    : std::move(previousTargets.at(index));

            auto previousTtargetTelemetry = previousTtarget->getTelemetry();
            previousTtarget->setTelemetry({.position = currentTtargetTelemetry.position, .speed = previousTtargetTelemetry.speed});

            previousTargetsTimes.insert_or_assign(index, currentTime);
            previousTargets.insert_or_assign(index, std::move(previousTtarget));
        }

        currentTargetsTimes.insert_or_assign(index, time);
        currentTarget->setTelemetry({.position = pos, .speed = currentTtargetTelemetry.speed});
    }

    if (currentTargets.contains(index) && previousTargets.contains(index)) {
        auto dt = currentTargetsTimes.at(index) - previousTargetsTimes.at(index);

        auto currPos = currentTargets.at(index).get()->getTelemetry().position;
        auto prevPos = previousTargets.at(index).get()->getTelemetry().position;

        currentTargets.at(index).get()->setTelemetry(
            {.position = currPos, .speed = {.x = (currPos.x - prevPos.x) / dt, .y = (currPos.y - prevPos.y) / dt}});
    }
}
