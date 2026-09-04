#include "TestFilesRepository.hpp"
#include <filesystem>

TestFilesRepository::TestFilesRepository(std::string fileRepositoryPath)
    : fileRepositoryPath(std::move(fileRepositoryPath))
{
}

auto TestFilesRepository::setTest(TestCode test) -> void
{
    this->test = test;
}

auto TestFilesRepository::getConfigFile() -> std::ifstream
{
    auto configFilePath = fileRepositoryPath + "/tests/t" + std::to_string(static_cast<int>(test)) + "/config.json";

    return std::ifstream{configFilePath};
}

auto TestFilesRepository::getTargetsFile() -> std::ifstream
{
    auto targetsFilePath = fileRepositoryPath + "/tests/t" + std::to_string(static_cast<int>(test)) + "/targets.json";

    return std::ifstream{targetsFilePath};
}

auto TestFilesRepository::putSimulationFile() -> std::ofstream
{
    auto simulationFilePath = fileRepositoryPath + "/tests/t" + std::to_string(static_cast<int>(test)) + "/simulation.json";

    return std::ofstream{simulationFilePath};
}

auto TestFilesRepository::getSimulationFile() -> std::ifstream
{
    auto simulationFilePath = fileRepositoryPath + "/tests/t" + std::to_string(static_cast<int>(test)) + "/simulation.json";

    return std::ifstream{simulationFilePath};
}

auto TestFilesRepository::hasSimulationFile() -> bool
{
    auto simulationFilePath = fileRepositoryPath + "/tests/t" + std::to_string(static_cast<int>(test)) + "/simulation.json";

    return std::filesystem::exists(simulationFilePath);
}
