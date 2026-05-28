#include "JsonTargetProvider.hpp"
#include "ITargetsProvider.hpp"
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

JsonTargetProvider::JsonTargetProvider(const char* jsonFilePath)
  : jsonFilePath(jsonFilePath)
{
}

void JsonTargetProvider::load()
{
    std::ifstream jsonFile{jsonFilePath};

    if (!jsonFile.is_open()) {
        throw std::runtime_error("Unable to open JSON file");
    }

    try {
        json targetsJson = json::parse(jsonFile);

        targetsCount = targetsJson["targetCount"];
        timeSteps = targetsJson["timeSteps"];
        targets = new Target[targetsCount];

        for (int i = 0; i < targetsCount; i++) {
            auto targetCoords = new Coord[timeSteps];

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

Target JsonTargetProvider::getTarget(int index)
{
    if (index < 0 || index >= targetsCount) {
        throw std::out_of_range("Target index out of range");
    }

    return targets[index];
}

int JsonTargetProvider::getTargetsCount()
{
    return targetsCount;
}

int JsonTargetProvider::getTimeSteps()
{
    return timeSteps;
}

JsonTargetProvider::~JsonTargetProvider()
{
    delete[] targets;
}