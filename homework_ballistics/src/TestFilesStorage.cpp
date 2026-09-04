#include "TestFilesStorage.hpp"
#include <httplib.h>
#include <format>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

void from_json(const json& j, TestFlleInfo& fileInfo)
{
    fileInfo.found = j["found"].get<bool>();

    if (fileInfo.found) {
        fileInfo.studentId = j["studentId"].get<std::string>();
        fileInfo.testId = j["testId"].get<std::string>();
        fileInfo.uploadedAt = j["uploadedAt"].get<std::string>();
        fileInfo.steps = j["steps"].get<int>();
    }
}

TestFilesStorage::TestFilesStorage(std::string studentId, TestsStorageConfig testsStorageConfig)
    : studentId(std::move(studentId))
    , testsStorageConfig(std::move(testsStorageConfig))
{
}

auto TestFilesStorage::send(TestCode testCode, std::ifstream simulationFile) -> void
{
    int tries = 0;
    DEBUG("Sending simulation file. Student ID: " << studentId << ", Test Code: " << static_cast<int>(testCode)
                                                  << ", Storage URL: " << testsStorageConfig.url);

    auto client = createClient();

    std::string getUrl = std::format("/api/dz12/results/T{:02}/{}", static_cast<int>(testCode), studentId);
    std::string saveUrl = "/api/dz12/results";
    auto fileContent = loadSimulationContent(testCode, simulationFile);

    simulationFile.close();

    bool isSaved = false;
    bool isError = false;

    int statusCode = 0;

    try {
        while (tries < 5 && !isSaved && !isError) {
            auto postResult = client.Post(saveUrl, fileContent, "application/json");
            tries++;
            statusCode = postResult->status;

            bool isSent = false;
            bool isRetry = false;

            switch (statusCode) {
                case 200:
                case 201: {
                    isSent = true;

                    break;
                }
                case 400:
                case 401:
                case 404:
                    isError = true;

                    break;
                // 502, 503...
                default:
                    isRetry = true;

                    break;
            }

            // check file saved
            if (isSent) {
                auto getResult = client.Get(getUrl);

                if (getResult->status == 200) {
                    auto fileInfo = parseFileInfo(getResult->body);

                    if (fileInfo.found) {
                        isSaved = true;
                    }
                }
            }

            if (isRetry) {
                std::this_thread::sleep_for(std::chrono::seconds(retryDelay));
            }
        }
    }
    catch (const std::exception& ex) {
        statusCode = 500;
        DEBUG("Error sending simulation file: " << ex.what());
    }

    LOG("Test: " << std::to_string(testCode) << " Status: " << std::to_string(statusCode) << " Tries: " << std::to_string(tries));
}

auto TestFilesStorage::loadSimulationContent(TestCode testCode, std::ifstream& simulationFile) -> std::string
{
    json simulationJson;
    json wrapperJson;

    try {
        simulationFile >> simulationJson;
        wrapperJson["studentId"] = studentId;
        wrapperJson["testId"] = std::format("T{:02}", static_cast<int>(testCode));
        wrapperJson["simulation"] = simulationJson;
    }
    catch (const std::exception& ex) {
        throw std::runtime_error(std::string("Error parsing config file: ") + ex.what());
    }

    return wrapperJson.dump(2);
}

auto TestFilesStorage::parseFileInfo(const std::string& fileInfoContent) -> TestFlleInfo
{
    try {
        auto fileInfoJson = json::parse(fileInfoContent);

        return fileInfoJson.get<TestFlleInfo>();
    }
    catch (const std::exception& ex) {
        throw std::runtime_error(std::string("Error parsing config file: ") + ex.what());
    }
}

auto TestFilesStorage::createClient() -> httplib::Client
{
    httplib::Client cli(testsStorageConfig.url);
    cli.set_connection_timeout(testsStorageConfig.connectionTimeout);
    cli.set_read_timeout(testsStorageConfig.readTimeout, 0);
    cli.set_write_timeout(testsStorageConfig.writeTimeout, 0);
    cli.set_default_headers({{"x-api-key", testsStorageConfig.apiKey}});

    return cli;
}
