#pragma once

#include "common.hpp"
#include "AmmoProvider.hpp"

struct DroneConfig {
    Coord startPos;          // початкова позиція (x, y)
    float altitude;          // висота
    float initialDir;        // початковий напрямок (рад)
    float attackSpeed;       // швидкість атаки (м/с)
    float accelerationPath;  // шлях розгону (м)
    AmmoParams ammo;         // обрані боєприпаси
    float arrayTimeStep;     // крок часу масиву цілей
    float simTimeStep;       // крок симуляції
    float hitRadius;         // радіус влучення
    float angularSpeed;      // кутова швидкість (рад/с)
    float turnThreshold;     // поріг повороту (рад)

    float acceleration() const { return attackSpeed * attackSpeed / (2 * accelerationPath); }
    float fullAccelerationTime() const { return attackSpeed / acceleration(); }
    float angleStep() const { return angularSpeed * simTimeStep; }
};

class IConfigLoader {
  public:
    virtual void load() = 0;
    virtual DroneConfig getConfig() = 0;
    virtual ~IConfigLoader() = default;
};