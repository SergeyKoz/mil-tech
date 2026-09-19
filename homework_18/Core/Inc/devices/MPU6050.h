#ifndef SRC_MPU6050_H_
#define SRC_MPU6050_H_

#define MPU6050_ADDR 0xD0
#define PWR_MGMT_1_REG 0x6B
#define ACCEL_CONFIG_REG 0x1C
#define GYRO_CONFIG_REG 0x1B
#define ACCEL_XOUT_H_REG 0x3B
#define GYRO_XOUT_H_REG 0x43

#include "main.h" // Потрібно для доступу до функцій HAL
#include <math.h>
#include <cstdio>

struct MPU6050_t {
    int16_t Accel_X_RAW;
    int16_t Accel_Y_RAW;
    int16_t Accel_Z_RAW;
    int32_t Ax;
    int32_t Ay;
    int32_t Az;
    int16_t Gyro_X_RAW;
    int16_t Gyro_Y_RAW;
    int16_t Gyro_Z_RAW;
    int32_t Gx;
    int32_t Gy;
    int32_t Gz;

    int toJson(char* buffer, size_t bufferLen) {
        int axInt  = Ax / 100;
        int axFrac = std::abs(Ax % 100);

        int ayInt  = Ay / 100;
        int ayFrac = std::abs(Ay % 100);

        int azInt  = Az / 100;
        int azFrac = std::abs(Az % 100);


        int gxInt  = Gx / 100;
        int gxFrac = std::abs(Gx % 100);

        int gyInt  = Gy / 100;
        int gyFrac = std::abs(Gy % 100);

        int gzInt  = Gz / 100;
        int gzFrac = std::abs(Gz % 100);

        return snprintf(
            buffer, bufferLen,
            "{\"device\": \"mpu5060\", \"data\": {\"ax\": %d.%02d, \"ay\": %d.%02d, \"az\": %d.%02d, \"gx\": %d.%02d, \"gy\": %d.%02d, \"gz\": %d.%02d}}\n",
            axInt, axFrac, ayInt, ayFrac, azInt, azFrac, gxInt, gxFrac, gyInt, gyFrac, gzInt, gzFrac
        );
    };
};

class MPU6050 {
private:
    I2C_HandleTypeDef& i2cBus;
    MPU6050_t mpu6050Data{};

public:
    MPU6050(I2C_HandleTypeDef& i2cBus);

    auto init() -> bool;
    auto read() -> MPU6050_t;
};

#endif /* SRC_MPU6050_H_ */