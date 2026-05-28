#include "JsonSimulationExport.hpp"
#include "MissionProcessor.hpp"
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

JsonSimulationExport::JsonSimulationExport(const char* jsonFilePath)
  : jsonFilePath(jsonFilePath)
{
}

void JsonSimulationExport::dumpResults(int steps, const SimStep* results)
{
    json outData;
    outData["totalSteps"] = steps;
    outData["steps"] = json::array();
    for (int i = 0; i < steps; i++) {
        json step;
        step["position"] = {{"x", results[i].pos.x}, {"y", results[i].pos.y}};
        step["direction"] = results[i].direction;
        step["state"] = results[i].state;
        step["targetIndex"] = results[i].targetIdx;
        step["dropPoint"] = {{"x", results[i].dropPoint.x}, {"y", results[i].dropPoint.y}};
        step["aimPoint"] = {{"x", results[i].aimPoint.x}, {"y", results[i].aimPoint.y}};
        step["predictedTarget"] = {{"x", results[i].predictedTarget.x}, {"y", results[i].predictedTarget.y}};
        outData["steps"].push_back(step);
    }
    std::ofstream simulationFile(jsonFilePath);

    if (!simulationFile.is_open()) {
        throw std::runtime_error("Unable to open JSON file for writing");
    }

    simulationFile << outData.dump(2);
    simulationFile.close();
}
