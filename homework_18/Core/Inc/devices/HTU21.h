#ifndef SRC_HTU21_H_
#define SRC_HTU21_H_

#include <cstddef>
#include <cstdio>
#include <cmath>

#define HTU21D_I2C_ADDR (0x40 << 1) // 0x80

// Команди HTU21D
#define HTU21D_CMD_MEAS_TEMP 0xF3 // Trigger Temperature Measurement (No Hold Master)
#define HTU21D_CMD_MEAS_HUMI 0xF5 // Trigger Humidity Measurement (No Hold Master)
#define HTU21D_CMD_RESET 0xFE // Soft Reset

#include "main.h" 

struct HTU21_t {
    int temperature;
    int humidity;

    int toJson(char* buffer, size_t bufferLen) {
        int temp_int  = temperature / 100;
        int temp_frac = std::abs(temperature % 100);

        int hum_int  = humidity / 100;
        int hum_frac = std::abs(humidity % 100);

        return snprintf(
            buffer, bufferLen,
            "{\"device\": \"htu21\", \"data\": {\"temperature\": %d.%02d, \"humidity\": %d.%02d}}\n",
            temp_int, temp_frac, hum_int, hum_frac
        );
    };
};

class HTU21 {
public:
    HTU21(I2C_HandleTypeDef& i2cBus);

    auto init() -> bool;
    auto read() -> HTU21_t;

private:
    I2C_HandleTypeDef& i2cBus;
    HTU21_t htu21Data{};

    HAL_StatusTypeDef readTemperature(int *temperature);
    HAL_StatusTypeDef readHumidity(int *humidity);
};

#endif /* SRC_BMP280_H_ */