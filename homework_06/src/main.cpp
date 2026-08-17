#include "../include/ballistics.hpp"
#include <iostream>
#include <fstream>

auto main(int argc, char** argv) -> int
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>\n";

        return 1;
    }

    float xd = 0.0F;
    float yd = 0.0F;
    float zd = 0.0F;
    float targetX = 0.0F;
    float targetY = 0.0F;
    float attackSpeed = 0.0F;
    float accelerationPath = 0.0F;

    std::string inputAmmoName;
    std::ifstream input(argv[1]);
    input >> xd >> yd >> zd >> targetX >> targetY >> attackSpeed >> accelerationPath >> inputAmmoName;

    const char* ammoName = inputAmmoName.c_str();

    try {
        BallisticParameters drop_params = calc_ballistic_parameters(ammoName, attackSpeed, zd);
        FireParameters fire_params = calc_fire_parameters(drop_params, {xd, yd}, {targetX, targetY}, accelerationPath);

        std::cout << "Drop time: " << drop_params.time << " seconds\n";
        std::cout << "Drop distance: " << drop_params.distance << " meters\n";
        std::cout << "Target distance: " << fire_params.distance << " meters\n";
        std::cout << "Fire coordinates: (" << fire_params.fire.x << ", " << fire_params.fire.y << ")\n";
        std::cout << "Intermediate coordinates: (" << fire_params.intermediate.x << ", " << fire_params.intermediate.y << ")\n";
    }
    catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << "\n";

        return 1;
    }

    return 0;
}