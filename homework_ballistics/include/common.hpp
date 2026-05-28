#pragma once

#include <cmath>

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

    Coord move(float distance, float direction) { return {x + distance * std::cos(direction), y + distance * std::sin(direction)}; }

    float distance(const Coord& other) const { return std::sqrt(std::pow((other.x - x), 2) + std::pow((other.y - y), 2)); }

    float direction(const Coord& other) const
    {
        float direction = std::atan2(other.y - y, other.x - x);

        if (direction < 0) {
            direction += 2.0f * M_PI;
        }

        return direction;
    }
};

enum DroneStatus { STOPPED = 0, ACCELERATING = 1, DECELERATING = 2, TURNING = 3, MOVING = 4 };