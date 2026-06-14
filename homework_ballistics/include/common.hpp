#pragma once

#include <cmath>
#include <string>
#include <vector>

constexpr float epsilon = 1e-5f;

struct Coord {
    float x;
    float y;

    Coord operator+(const Coord& other) const
    {
        Coord result;
        result.x = x + other.x;
        result.y = y + other.y;
        return result;
    }

    Coord operator-(const Coord& other) const
    {
        Coord result;
        result.x = x - other.x;
        result.y = y - other.y;
        return result;
    }

    Coord operator*(float scalar) const { return {x * scalar, y * scalar}; }

    Coord operator/(float scalar) const { return {x / scalar, y / scalar}; }

    bool operator==(const Coord& other) const
    {
        const float epsilon = 1e-5f;

        return std::abs(x - other.x) < epsilon && std::abs(y - other.y) < epsilon;
    }

    bool operator!=(const Coord& other) const { return !(*this == other); }

    Coord move(float distance, float direction) { return {x + distance * std::cos(direction), y + distance * std::sin(direction)}; }

    float distance(const Coord& other) const { return std::sqrt(std::pow((other.x - x), 2) + std::pow((other.y - y), 2)); }

    float direction(const Coord& other) const
    {
        float direction = std::atan2(other.y - y, other.x - x);

        if (direction < 0) {
            direction += 2.0F * M_PI;
        }

        return direction;
    }
};

struct AmmoParams {
    std::string name;
    float mass;  // маса (кг)
    float drag;  // коефіцієнт опору
    float lift;  // коефіцієнт підйому
};

struct DropParameters {
    float time;      // час падіння
    float distance;  // відстань, на яку відхилиться снаряд від вертикалі
};

enum DroneStatus { STOPPED = 0, ACCELERATING = 1, DECELERATING = 2, TURNING = 3, MOVING = 4 };

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

struct Target {
    std::vector<Coord> positions;
    Coord interpolate(float time, float timeStep)
    {
        int index = (int)floor(time / timeStep);
        int _current = index % 60;
        int _next = (_current + 1) % 60;
        float frac = (time - index * timeStep) / timeStep;
        Coord current = positions[_current];
        Coord next = positions[_next];

        return {current.x + (next.x - current.x) * frac, current.y + (next.y - current.y) * frac};
    };
};

struct SimStep {
    Coord pos;          // позиція дрона
    float direction;    // напрямок (рад)
    DroneStatus state;  // стан автомата (0-4)
    int targetIdx;      // індекс поточної цілі
    Coord dropPoint;    // точка скиду (куди летить дрон)
    Coord aimPoint;     // куди впаде бомба (якщо скинути зараз)
    Coord predictedTarget;
    float speed;

    void turn(float angle)
    {
        direction += angle;

        direction = std::fmod(direction, 2.0f * M_PI);

        if (direction < 0.0f) {
            direction += 2.0f * M_PI;
        }
    }
};

struct SelectedTarget {
    int idx;
    Target* target;
    Coord position;
    float timeToReachPosition;
};

struct DroneContext {
    float currentTime;
    SimStep* simulationStep;
    DroneConfig* droneConfig;
    DropParameters* dropParams;
    float turnAngle;
    float acceleration;
    float angleStep;
    float distanceToDropPoint;

    void calcTelemetry(SelectedTarget selectedTarget)
    {
        simulationStep->targetIdx = selectedTarget.idx;

        Coord interpolatedPos = selectedTarget.target->interpolate(currentTime + droneConfig->simTimeStep, droneConfig->arrayTimeStep);
        Coord delta = interpolatedPos - selectedTarget.position;

        float vx = delta.x / droneConfig->simTimeStep;
        float vy = delta.y / droneConfig->simTimeStep;

        simulationStep->predictedTarget = {
            selectedTarget.position.x + vx * selectedTarget.timeToReachPosition,
            selectedTarget.position.y + vy * selectedTarget.timeToReachPosition,
        };

        float targetDistance = simulationStep->pos.distance(interpolatedPos);
        simulationStep->dropPoint = simulationStep->pos.move(targetDistance - dropParams->distance, simulationStep->direction);
        simulationStep->aimPoint = simulationStep->pos.move(dropParams->distance, simulationStep->direction);

        distanceToDropPoint = simulationStep->pos.distance(simulationStep->predictedTarget);
    }
};