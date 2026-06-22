#include "export/JsonSimulationExport.hpp"
// #include "MissionProcessor.hpp"
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

JsonSimulationExport::JsonSimulationExport(std::string jsonFilePath)
    : jsonFilePath(std::move(jsonFilePath))
{
}

void JsonSimulationExport::dumpResults(int steps, std::array<SimStep, ThreadMissionProcessor::MAX_STEPS>& results)
{
    json outData;
    outData["totalSteps"] = steps - 1;
    outData["steps"] = json::array();
    int i = 0;

    for (auto& simStep : results) {
        if (i > steps - 1) {
            break;
        }

        json step;
        step["position"] = {{"x", simStep.pos.x}, {"y", simStep.pos.y}};
        step["direction"] = simStep.direction;
        step["state"] = simStep.state;
        step["step"] = i;
        step["targetIndex"] = simStep.targetIdx;
        step["dropPoint"] = {{"x", simStep.dropPoint.x}, {"y", simStep.dropPoint.y}};
        step["aimPoint"] = {{"x", simStep.aimPoint.x}, {"y", simStep.aimPoint.y}};
        step["predictedTarget"] = {{"x", simStep.predictedTarget.x}, {"y", simStep.predictedTarget.y}};
        step["timeSecSinceStart"] = simStep.timeSecSinceStart;
        outData["steps"].push_back(step);

        i++;
    }
    std::ofstream simulationFile(jsonFilePath);

    if (!simulationFile.is_open()) {
        throw std::runtime_error("Unable to open JSON file for writing");
    }

    simulationFile << outData.dump(2);
    simulationFile.close();
}
