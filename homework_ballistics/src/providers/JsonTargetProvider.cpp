#include "providers/JsonTargetProvider.hpp"
#include "common.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <vector>

using json = nlohmann::json;

JsonTargetProvider::JsonTargetProvider(std::string jsonFilePath)
    : jsonFilePath(std::move(jsonFilePath))
    , targetsCount(0)
    , timeSteps(0)
    , targets({})
{
}

void JsonTargetProvider::load()
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

            targets[i] = {.positions = targetCoords};
        }
    }
    catch (const std::exception& ex) {
        throw std::runtime_error(std::string("Error parsing JSON file: ") + ex.what());
    }

    jsonFile.close();
}

auto JsonTargetProvider::getTarget(int index) -> Target
{
    if (index < 0 || index >= targetsCount) {
        throw std::out_of_range("Target index out of range");
    }

    return targets.at(index);
}

auto JsonTargetProvider::getTargetsCount() -> int
{
    return targetsCount;
}

auto JsonTargetProvider::getTimeSteps() -> int
{
    return timeSteps;
}
