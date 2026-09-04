#include <iostream>
#include "DroneAutopilot.hpp"
#include "RpiMissionFactory.hpp"
#include "rpiChecker/RpiCheckerGPIO.hpp"
#include "rpiChecker/RpiCheckerUART.hpp"
#include "CliParams.hpp"

auto main(int argc, char *argv[]) -> int
{
    try {
        auto cliParams = CliParams::parse(argc, argv);

        auto configLoader = RpiMissionFactory::createConfigLoader(ConfigLoaderType::CHECKER, AmmoLoaderType::NONE);
        auto targetProvider = RpiMissionFactory::createTargetsProvider(ProviderType::CHECKER);
        auto ballisticsSolver = RpiMissionFactory::createBallisticsSolver(SolverType::TABLE);
        auto rpiCheckerGPIO = std::make_unique<RpiCheckerGPIO>(cliParams.gpioChip, cliParams.startLine, cliParams.dropLine);
        auto rpiCheckerUART = std::make_unique<RpiCheckerUART>(cliParams.uartPort);

        auto autopilot = std::make_unique<DroneAutopilot>(std::move(ballisticsSolver),
                                                          std::move(configLoader),
                                                          std::move(targetProvider),
                                                          std::move(rpiCheckerGPIO),
                                                          std::move(rpiCheckerUART));
        autopilot->init();
        autopilot->start();
        autopilot->mission();
        autopilot->drop();

        LOG("Mission completed successfully.");
    }
    catch (const std::runtime_error &ex) {
        std::cerr << ex.what() << '\n';

        return 1;
    }
    catch (const std::invalid_argument &ex) {
        std::cerr << ex.what() << '\n';

        return 1;
    }
    catch (const std::exception &ex) {
        std::cerr << ex.what() << '\n';

        return 1;
    }

    return 0;
}