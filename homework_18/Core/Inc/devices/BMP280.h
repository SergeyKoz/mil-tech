#ifndef SRC_BMP280_H_
#define SRC_BMP280_H_

#define SEALEVELPRESSURE_PA 101325

#include "main.h"
#include <cstdio>
#include <cmath>

struct BMP280_t {
    int temperature;
    int pressure_pa;
    int pressure_hpa;
    int altitude_m;

    int toJson(char* buffer, size_t bufferLen) {
        int temperatureInt  = temperature / 100;
        int temperatureFrac = std::abs(temperature % 100);

        int pressureInt  = pressure_hpa / 100;
        int pressureFrac = std::abs(pressure_hpa % 100);

        int altitudeInt  = altitude_m / 100;
        int altitudeFrac = std::abs(altitude_m % 100);

        return snprintf(
            buffer, bufferLen,
            "{\"device\": \"bmp280\", \"data\": {\"temperature\": %d.%02d, \"pressure\": %d.%02d, \"altitude\": %d.%02d}}\n",
            temperatureInt, temperatureFrac, pressureInt, pressureFrac, altitudeInt, altitudeFrac
        );
    };
};

class BMP280 {
public:
    BMP280(SPI_HandleTypeDef& spiBus);

    auto init() -> bool;
    auto read() -> BMP280_t;

private:
    SPI_HandleTypeDef& spiBus;
    BMP280_t bmp280Data{};

    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;

    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;

    int32_t t_fine; 

    void writeReg(uint8_t reg, uint8_t value);
    void readRegs(uint8_t reg, uint8_t *pData, uint16_t length);
    void readRawData(int32_t *raw_temp, int32_t *raw_press);
    void readTrimmingParameters();
    int32_t compensateT(int32_t adc_T);
    uint32_t compensateP(int32_t adc_P);
    int calculateAltitude(int pressure_pa);    
};

#endif /* SRC_BMP280_H_ */