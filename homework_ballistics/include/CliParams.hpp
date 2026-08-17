#pragma once

struct CliParams {
    const char *uartPort = nullptr;
    const char *gpioChip = nullptr;
    int startLine = -1;
    int dropLine = -1;

    static auto parse(int argc, char *argv[]) -> CliParams
    {
        auto tryParseInt = [](const char *str, int &outValue) -> bool {
            if (!str || *str == '\0') {
                return false;
            }

            char *endPtr = nullptr;
            long val = std::strtol(str, &endPtr, 10);

            if (*endPtr != '\0') {
                return false;
            }

            outValue = static_cast<int>(val);

            return true;
        };

        CliParams params;

        for (int i = 1; i < argc; ++i) {
            std::string_view arg(argv[i]);

            if (arg == "--uart") {
                if (i + 1 < argc) {
                    params.uartPort = argv[++i];
                }
                else {
                    throw std::invalid_argument("Parameter --uart is required");
                }
            }
            else if (arg == "--gpiochip") {
                if (i + 1 < argc) {
                    params.gpioChip = argv[++i];
                }
                else {
                    throw std::invalid_argument("Error: --gpiochip requires a path value");
                }
            }
            else if (arg == "--start-line") {
                if (i + 1 < argc) {
                    const char *val = argv[++i];

                    if (!tryParseInt(val, params.startLine)) {
                        throw std::invalid_argument("Error: --start-line must be a valid integer. Got: " + std::string(val));
                    }
                }
                else {
                    throw std::invalid_argument("Error: --start-line requires a numeric value.");
                }
            }
            else if (arg == "--drop-line") {
                if (i + 1 < argc) {
                    const char *val = argv[++i];

                    if (!tryParseInt(val, params.dropLine)) {
                        throw std::invalid_argument("Error: --drop-line must be a valid integer. Got: " + std::string(val));
                    }
                }
                else {
                    throw std::invalid_argument("Error: --drop-line requires a numeric value.");
                }
            }
            else {
                throw std::invalid_argument("Error: Unknown argument" + std::string(arg));
            }
        }

        return params;
    };
};