#include "../include/ballistics.hpp"
#include <iostream>

auto main(int argc, char** argv) -> int
{
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <ammo_name> <initial_velocity> <initial_height>\n";

        return 1;
    }

    const char* ammoName = argv[1];
    float v0 = std::stof(argv[2]);
    float z0 = std::stof(argv[3]);

    DropParameters drop_params = calc_drop_parameters(ammoName, v0, z0);

    std::cout << "Drop time: " << drop_params.time << " seconds\n";
    std::cout << "Drop distance: " << drop_params.distance << " meters\n";

    return 0;
}