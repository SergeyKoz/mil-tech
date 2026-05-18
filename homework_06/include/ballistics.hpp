#pragma once

const float GRAVITY = 9.81F;
const int NAME_LENGTH = 32;
const int AMMO_COUNT = 5;

struct AmmoParams {
    char name[NAME_LENGTH];
    float mass;  // маса (кг)
    float drag;  // коефіцієнт опору
    float lift;  // коефіцієнт підйому
};

const AmmoParams ammoList[AMMO_COUNT] = {{.name = "VOG-17", .mass = 0.35F, .drag = 0.07F, .lift = 0.0F},
                                         {.name = "M67", .mass = 0.60F, .drag = 0.10F, .lift = 0.0F},
                                         {.name = "RKG-3", .mass = 1.20F, .drag = 0.10F, .lift = 0.0F},
                                         {.name = "GLIDING-VOG", .mass = 0.45F, .drag = 0.10F, .lift = 1.0F},
                                         {.name = "GLIDING-RKG", .mass = 1.40F, .drag = 0.10F, .lift = 1.0F}};

struct DropParameters {
    float time;      // час падіння
    float distance;  // відстань, на яку відхилиться снаряд від вертикалі
};

auto calc_drop_parameters(const char ammo_name[32], float v0, float z0) -> DropParameters;
