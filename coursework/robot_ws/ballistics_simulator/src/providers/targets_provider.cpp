#include "ballistics_simulator/providers/targets_provider.hpp"
// #include "common.hpp"
#include <memory>
#include <vector>
// #include "Target.hpp"

namespace ballistics_simulator
{

    // void CheckerTargetProvider::load() {}

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

    // auto CheckerTargetProvider::getTimeSteps() -> int
    // {
    //     return 0;
    // }

    auto TargetsProvider::setTarget(int index, Coord pos, float time) -> void
    {
        // struct TargetTelemetry
        // {
        //     Coord position;
        //     Speed speed;
        // };

        // if (index == 2)
        // {
        //     log() << "setTarget pos: " << pos.x << "," << pos.y;
        // }
        // log() << "currentTargets.size=" << currentTargets.size() << " previousTargets.size=" << previousTargets.size() << " time=" << time;

        if (!currentTargets.contains(index))
        {
            // auto curentTarget = std::make_unique<TargetTelemetry>(std::vector<Coord>{pos}, 0);
            // currentTargetsTimes.insert_or_assign(index, time);
            // currentTargets.insert_or_assign(index, std::move(curentTarget));

            // auto curentTarget = std::make_unique<TargetTelemetry>(std::vector<Coord>{pos}, 0);
            TargetTelemetry curentTarget = {.position = pos, .speed = {}};
            currentTargetsTimes.insert_or_assign(index, time);
            currentTargets.insert_or_assign(index, curentTarget);
        }
        else
        {
            auto currentTime = currentTargetsTimes.at(index);
            // auto *currentTarget = currentTargets.at(index).get();
            // auto currentTtargetTelemetry = currentTarget->getTelemetry();
            auto currentTarget = currentTargets.at(index);
            // auto currentTtargetTelemetry = currentTarget->getTelemetry();

            if (std::abs(currentTime - time) > epsilon)
            {

                // auto previousTtarget = !previousTargets.contains(index) ? std::make_unique<TargetTelemetry>(std::vector<Coord>{Coord{}}, 0)
                //                                                         : std::move(previousTargets.at(index));

                // auto previousTtargetTelemetry = previousTtarget->getTelemetry();
                // previousTtarget->setTelemetry({.position = currentTtargetTelemetry.position, .speed = previousTtargetTelemetry.speed});

                // previousTargetsTimes.insert_or_assign(index, currentTime);
                // previousTargets.insert_or_assign(index, std::move(previousTtarget));
                ////
                auto previousTarget = !previousTargets.contains(index) ? TargetTelemetry{} : previousTargets.at(index);

                // previousTtarget.position = currentTarget.position;

                previousTarget = {.position = currentTarget.position, .speed = previousTarget.speed};

                previousTargetsTimes.insert_or_assign(index, currentTime);
                // previousTargets.insert_or_assign(index, previousTtarget);

                // if (index == 2)
                // {
                //     log() << "set prev: pos: " << currentTarget.position.x << "," << currentTarget.position.y;
                // }

                previousTargets.insert_or_assign(index, TargetTelemetry{.position = currentTarget.position, .speed = previousTarget.speed});
            }

            // currentTargetsTimes.insert_or_assign(index, time);
            // currentTarget->setTelemetry({.position = pos, .speed = currentTtargetTelemetry.speed});

            currentTargetsTimes.insert_or_assign(index, time);
            // currentTarget.position = pos;
        }

        if (currentTargets.contains(index) && previousTargets.contains(index))
        {
            auto dt = currentTargetsTimes.at(index) - previousTargetsTimes.at(index);

            // auto currPos = currentTargets.at(index).position;
            auto prevPos = previousTargets.at(index).position;

            // if (index == 2)
            // {
            //     log() << "dt: " << dt << " currPos: " << pos.x << "," << pos.y << " prevPos: " << prevPos.x << "," << prevPos.y;
            // }

            currentTargets.at(index) = {.position = pos, .speed = {.x = (pos.x - prevPos.x) / dt, .y = (pos.y - prevPos.y) / dt}};
        }

        // if (index == 2)
        // {
        //     log() << "Targets set";
        // }

        // if (currentTargets.contains(index) && previousTargets.contains(index))
        // {
        //     auto dt = currentTargetsTimes.at(index) - previousTargetsTimes.at(index);

        //     auto currPos = currentTargets.at(index).get()->getTelemetry().position;
        //     auto prevPos = previousTargets.at(index).get()->getTelemetry().position;

        //     currentTargets.at(index).get()->setTelemetry(
        //         {.position = currPos, .speed = {.x = (currPos.x - prevPos.x) / dt, .y = (currPos.y - prevPos.y) / dt}});
        // }
    }
} // namespace ballistics_simulator
