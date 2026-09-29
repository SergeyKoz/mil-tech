#pragma once

#include <cmath>
#include <string>

namespace ballistics_simulator
{
    constexpr float epsilon = 1e-4f;

    struct Coord
    {
        float x;
        float y;

        Coord operator+(const Coord &other) const
        {
            Coord result;
            result.x = x + other.x;
            result.y = y + other.y;
            return result;
        }

        Coord operator-(const Coord &other) const
        {
            Coord result;
            result.x = x - other.x;
            result.y = y - other.y;
            return result;
        }

        Coord operator*(float scalar) const { return {x * scalar, y * scalar}; }

        Coord operator/(float scalar) const { return {x / scalar, y / scalar}; }

        bool operator==(const Coord &other) const
        {
            return std::abs(x - other.x) < epsilon && std::abs(y - other.y) < epsilon;
        }

        bool operator!=(const Coord &other) const { return !(*this == other); }

        Coord move(float distance, float direction) { return {x + distance * std::cos(direction), y + distance * std::sin(direction)}; }

        float distance(const Coord &other) const { return std::sqrt(std::pow((other.x - x), 2) + std::pow((other.y - y), 2)); }

        float direction(const Coord &other) const
        {
            float direction = std::atan2(other.y - y, other.x - x);

            if (direction < 0)
            {
                direction += 2.0F * M_PI;
            }

            return direction;
        }
    };

    struct Speed
    {
        float x;
        float y;

        float toSpeed() const { return std::sqrt(x * x + y * y); }

        Speed fromSpeed(float speed, float direction) const { return {.x = speed * std::cos(direction), .y = speed * std::sin(direction)}; }

        Speed &operator+=(const Speed &other)
        {
            x += other.x;
            y += other.y;

            return *this;
        }

        Speed operator+(const Speed &other) const
        {
            Speed result;
            result.x = x + other.x;
            result.y = y + other.y;

            return result;
        }

        Speed &operator-=(const Speed &other)
        {
            x -= other.x;
            y -= other.y;

            return *this;
        }

        Speed operator-(const Speed &other) const
        {
            Speed result;
            result.x = x - other.x;
            result.y = y - other.y;

            return result;
        }
    };

    struct AmmoConfig
    {
        std::string name;
        float mass;
        float drag;
        float lift;
        float hitRadius;
    };

    struct AmmoParams
    {
        std::string name;
        float mass; // маса (кг)
        float drag; // коефіцієнт опору
        float lift; // коефіцієнт підйому
    };

    struct DropParameters
    {
        float time;     // час падіння
        float distance; // відстань, на яку відхилиться снаряд від вертикалі
    };

    enum DroneStatus
    {
        STOPPED = 0,
        ACCELERATING = 1,
        DECELERATING = 2,
        TURNING = 3,
        MOVING = 4
    };

    struct DroneConfig
    {
        Coord startPos;         // початкова позиція (x, y)
        float altitude;         // висота
        float initialDir;       // початковий напрямок (рад)
        float attackSpeed;      // швидкість атаки (м/с)
        float accelerationPath; // шлях розгону (м)
        AmmoParams ammo;        // обрані боєприпаси
        float arrayTimeStep;    // крок часу масиву цілей
        float simTimeStep;      // крок симуляції
        float targetTimeStep;
        float physicsTimeStep;
        float timeScale;
        float hitRadius;     // радіус влучення
        float angularSpeed;  // кутова швидкість (рад/с)
        float turnThreshold; // поріг повороту (рад)

        float acceleration() const { return attackSpeed * attackSpeed / (2 * accelerationPath); }
        float fullAccelerationTime() const { return attackSpeed / acceleration(); }
    };

    struct SimStep
    {
        Coord pos;         // позиція дрона
        float direction;   // напрямок (рад)
        DroneStatus state; // стан автомата (0-4)
        int targetIdx;     // індекс поточної цілі
        Coord dropPoint;   // точка скиду (куди летить дрон)
        Coord aimPoint;    // куди впаде бомба (якщо скинути зараз)
        Coord predictedTarget;
        float speed;
        float timeSecSinceStart;
    };

    struct TargetTelemetry
    {
        Coord position;
        Speed speed;
    };

    struct SelectedTarget
    {
        int index;
        TargetTelemetry telemetry;
        float timeToReachPosition;
    };

    struct DroneTelemetry
    {
        DroneStatus state;
        Coord position;
        float altitude;
        Speed speed;
        float direction;
        float timeSinceStart;
    };

    struct DroneContext
    {
        float currentTime;
        SimStep *simulationStep;
        DroneConfig *droneConfig;
        DroneTelemetry droneTelemetry;
        DropParameters *dropParams;
        SelectedTarget selectedTarget;
        float turnAngle;
        float acceleration;
        float angleStep;
        float distanceToDropPoint;
    };

    struct DroneCommand
    {
        DroneStatus state;
        float angleSpeed;
        float acceleration;
        float maxSpeed;
    };

    struct ControlCommand
    {
        float acceleration; // прискорення вздовж курсу, [-1..1] (1 = повний газ, -1 = гальмо)
        float turnRate;     // швидкість повороту, [-1..1] (1 = макс. вліво, -1 = вправо)
    };
} // namespace ballistics_simulator