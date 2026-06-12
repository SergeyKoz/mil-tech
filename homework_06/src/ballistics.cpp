#define USE_MATH_DEFINES

#include "../include/ballistics.hpp"
#include <cmath>
#include <stdexcept>
#include <cstring>

auto calc_drop_time(const AmmoParams& ammo, float v0, float z0) -> float
{
    float a = ammo.drag * GRAVITY * ammo.mass - 2.0F * ammo.drag * ammo.drag * ammo.lift * v0;
    float b = -(3.0F * GRAVITY * ammo.mass * ammo.mass) + 3.0F * (ammo.drag * ammo.lift * ammo.mass * v0);
    float c = 6.0F * ammo.mass * ammo.mass * z0;

    // cardano

    float p = -(b * b) / (3.0F * a * a);
    float q = ((2.0F * b * b * b) / (27.0F * a * a * a)) + c / a;

    if (p >= 0) {
        throw std::runtime_error("Wrong p calc");
    }

    float acos_arg = 3.0F * q / (2.0F * p) * std::sqrt(-3.0F / p);

    if (acos_arg < -1 || acos_arg > 1) {
        throw std::runtime_error("Wrong acos calc");
    }

    float fi = std::acos(acos_arg);
    auto t = static_cast<float>((2.0F * std::sqrt(-p / 3.0F) * std::cos((fi + 4.0F * M_PI) / 3.0F)) - (b / (3.0F * a)));

    return t;
}

auto calc_drop_distance(float t, const AmmoParams& ammo, float v0) -> float
{
    float l = ammo.lift;
    float l2 = l * l;
    float l3 = l2 * l;
    float l4 = l3 * l;
    float ll2 = l2 + 1;

    float t2 = t * t;
    float t3 = t2 * t;
    float t4 = t3 * t;
    float t5 = t4 * t;

    float d = ammo.drag;
    float d2 = d * d;
    float d3 = d2 * d;
    float d4 = d3 * d;

    float m = ammo.mass;
    float m2 = m * m;
    float m3 = m2 * m;
    float m4 = m3 * m;

    float h1 = v0 * t;
    float h2 = (t2 * d * v0) / (2.0F * m);
    float h3 = t4 * ((3.0F * d3 * ll2 * l2 * v0) + (6.0F * d3 * l4 * ll2 * v0) - (6.0F * d2 * GRAVITY * (l4 + ll2) * l * m)) /
               (36.0F * ll2 * ll2 * m3);
    float h4 = t5 * ((3.0F * d3 * GRAVITY * l3 * m) - (3.0F * d4 * l2 * ll2 * v0)) / (36.0F * ll2 * m4);
    float h5 = t3 * ((6.0F * d * GRAVITY * l * m) - (6.0F * d2 * (l2 - 1) * v0)) / (36.0F * m2);

    return h1 - h2 + h3 + h4 + h5;
}

auto get_ammo_params(const char ammo_name[NAME_LENGTH]) -> AmmoParams
{
    for (const auto& ammo : ammoList) {
        if (std::strcmp(ammo_name, ammo.name) == 0) {
            return ammo;
        }
    }

    throw std::runtime_error("Wrong ammo name");
}

auto calc_ballistic_parameters(const char ammo_name[NAME_LENGTH], float v0, float z0) -> BallisticParameters
{
    if (v0 <= 0 || z0 <= 0) {
        throw std::runtime_error("Initial velocity and initial height must be positive");
    }

    AmmoParams ammo = get_ammo_params(ammo_name);
    float time = calc_drop_time(ammo, v0, z0);
    float distance = calc_drop_distance(time, ammo, v0);

    return {.time = time, .distance = distance};
}

auto calc_fire_parameters(BallisticParameters ballistic_params, Coord drone, Coord target, float accelerationPath) -> FireParameters
{
    auto distance = static_cast<float>(std::sqrt(std::pow((target.x - drone.x), 2) + std::pow((target.y - drone.y), 2)));
    auto ratio = (distance - ballistic_params.distance) / distance;

    Coord fire = {drone.x + (target.x - drone.x) * ratio, drone.y + (target.y - drone.y) * ratio};
    Coord intermediate = {0.0F, 0.0F};

    if (ballistic_params.distance + accelerationPath > distance) {
        intermediate = {target.x - (target.x - drone.x) * (ballistic_params.distance + accelerationPath) / distance,
                        target.y - (target.y - drone.y) * (ballistic_params.distance + accelerationPath) / distance};
    }

    return {distance, fire, intermediate};
}