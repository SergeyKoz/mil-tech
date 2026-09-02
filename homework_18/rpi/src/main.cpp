#include "common.hpp"
#include "UartReader.hpp"
#include "SensorController.hpp"
#include "GpioController.hpp"
#include "parser/JsonTelemetryParser.hpp"

auto main() -> int
{
    std::string uartPort = "/dev/ttyAMA0";
    std::string gpioChip = "gpiochip0";
    uint controlLine = 23;
    float temperatureLevel = 25.89f;
    int controlTaskInterval = 2000; // 2 seconds

    try
    {
        auto uartReader = std::make_unique<UartReader>(std::move(uartPort));
        auto gpioController = std::make_unique<GpioController>(std::move(gpioChip), controlLine);
        auto sensorsController = std::make_shared<SensorController>(std::chrono::milliseconds(controlTaskInterval), std::move(gpioController), temperatureLevel);
        auto jsonTelemetryParser = std::make_shared<JsonTelemetryParser>(sensorsController);

        sensorsController->init();

        uartReader->addParser(jsonTelemetryParser);
        uartReader->start();
        sensorsController->start();

        while (!uartReader->isThreadReady() || !sensorsController->isThreadReady())
        {
            std::this_thread::yield();
        }

        uartReader->wait();
        sensorsController->stop();

        return 0;
    }
    catch (const std::runtime_error &ex)
    {
        std::cerr << ex.what() << '\n';

        return 1;
    }
}