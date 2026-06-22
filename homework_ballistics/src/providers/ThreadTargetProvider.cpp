#include "providers/ThreadTargetProvider.hpp"
#include "common.hpp"
#include <memory>
#include <nlohmann/json.hpp>
#include <fstream>
#include <vector>
#include "Target.hpp"

using json = nlohmann::json;

ThreadTargetProvider::ThreadTargetProvider(std::string jsonFilePath, const DroneConfig& droneConfig)
    : IntervalWorker(std::chrono::milliseconds(static_cast<int>(std::round(droneConfig.simTimeStep * 1000))),
                     static_cast<int>(droneConfig.timeScale))
    , jsonFilePath(std::move(jsonFilePath))
    , targetsCount(0)
    , timeSteps(0)
    , arrayTimeStep(static_cast<int>(droneConfig.arrayTimeStep))
{
}

void ThreadTargetProvider::load()
{
    std::ifstream jsonFile{jsonFilePath};

    if (!jsonFile.is_open()) {
        throw std::runtime_error("Unable to open JSON file");
    }

    try {
        auto targetsJson = json::parse(jsonFile);

        targetsCount = targetsJson["targetCount"];
        timeSteps = targetsJson["timeSteps"];

        targets.resize(targetsCount);

        for (int i = 0; i < targetsCount; i++) {
            std::vector<Coord> targetCoords(timeSteps);

            for (int j = 0; j < timeSteps; j++) {
                targetCoords[j].x = targetsJson["targets"][i]["positions"][j]["x"];
                targetCoords[j].y = targetsJson["targets"][i]["positions"][j]["y"];
            }

            targets[i] = std::make_unique<Target>(targetCoords);
        }
    }
    catch (const std::exception& ex) {
        throw std::runtime_error(std::string("Error parsing JSON file: ") + ex.what());
    }

    jsonFile.close();
}

auto ThreadTargetProvider::getTarget(int index) -> Target*
{
    if (index < 0 || index >= targetsCount) {
        throw std::out_of_range("Target index out of range");
    }

    return targets.at(index).get();
}

auto ThreadTargetProvider::getTargetsCount() -> int
{
    return targetsCount;
}

auto ThreadTargetProvider::getTimeSteps() -> int
{
    return timeSteps;
}

auto ThreadTargetProvider::intervalTask() -> void
{
    auto timeStep = static_cast<float>(interval.count()) / 1000.0F;
    msSinceStart += static_cast<int>(interval.count());

    float timeSinceStart = static_cast<float>(msSinceStart) / 1000.0F;

    for (int i = 0; i < targetsCount - 1; i++) {
        auto* target = getTarget(i);

        target->update(timeSinceStart, static_cast<float>(arrayTimeStep), timeStep);
    }
}

ThreadTargetProvider::~ThreadTargetProvider() = default;
