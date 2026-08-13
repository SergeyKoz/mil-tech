#pragma once

#include <stdexcept>
#include <string>

struct CliParams {
    std::string source;
    // long address;
    int interval;

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

        if (argc < 3) {
            throw std::invalid_argument("Required arguments are not specified");
        }

        params.source = argv[1];

        if (!tryParseInt(argv[2], params.interval)) {
            throw std::invalid_argument("Cant parse interval arg: " + std::string(argv[2]));
        }
        // params.address = std::strtol(argv[2], nullptr, 16);

        return params;
    };
};