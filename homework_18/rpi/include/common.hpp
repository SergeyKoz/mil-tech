#pragma once

#define ENABLE_LOG 1
#define ENABLE_DEBUG 1

#if ENABLE_LOG
#define LOG(msg) std::cout << "[" << get_elapsed_ms() << "] " << msg << '\n'
#else
#define LOG(msg)
#endif

#if ENABLE_DEBUG
#define DEBUG(msg) std::cout << "[DEBUG] " << msg << '\n'
#else
#define DEBUG(msg)
#endif

#include <iostream>
#include <chrono>
#include <format>

inline long long get_elapsed_ms()
{
    static const auto start_time = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();

    return std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
}

struct MPU5060Data
{
    float ax;
    float ay;
    float az;
    float gx;
    float gy;
    float gz;

    std::string toString() const
    {
        return std::format(
            "MPU5060 ax={:.2f}, ay={:.2f}, az={:.2f}, gx={:.2f}, gy={:.2f}, gz={:.2f}",
            ax, ay, az, gx, gy, gz);
    };
};

struct BMP280Data
{
    float temperature;
    float pressure;
    float altitude;

    std::string toString() const
    {
        return std::format(
            "BMP280 temperature={:.2f}, pressure={:.2f}, altitude={:.2f}",
            temperature, pressure, altitude);
    };
};

struct HTU21Data
{
    float temperature;
    float humidity;

    std::string toString() const
    {
        return std::format(
            "HTU21 temperature={:.2f}, humidity={:.2f}",
            temperature, humidity);
    };
};