#include "devices/MPU6050.h"

MPU6050::MPU6050(I2C_HandleTypeDef& i2cBus) : i2cBus(i2cBus) {}

auto MPU6050::init() -> bool
{
    uint8_t check;
    uint8_t data;

    // Перевірка зв'язку (WHO_AM_I регістр 0x75 повертає 0x68)
    HAL_I2C_Mem_Read(&i2cBus, MPU6050_ADDR, 0x75, 1, &check, 1, 100);

    if (check == 0x68 || check == 0x40) {
        // Пробудження сенсора (запис 0 в PWR_MGMT_1)
        data = 0;
        HAL_I2C_Mem_Write(&i2cBus, MPU6050_ADDR, PWR_MGMT_1_REG, 1, &data, 1, 100);

        // Налаштування акселерометра (±2g, дільник 16384 LSB/g)
        data = 0x00;
        HAL_I2C_Mem_Write(&i2cBus, MPU6050_ADDR, ACCEL_CONFIG_REG, 1, &data, 1, 100);

        // Налаштування гіроскопа (±250 deg/s, дільник 131 LSB/deg/s)
        data = 0x00;
        HAL_I2C_Mem_Write(&i2cBus, MPU6050_ADDR, GYRO_CONFIG_REG, 1, &data, 1, 100);

        return true; // Успішно
    }

    return false; // Помилка
}

auto MPU6050::read() -> MPU6050_t {
    uint8_t Rec_Data[14];

    // Зчитуємо 14 послідовних байт починаючи з ACCEL_XOUT_H (0x3B)
    // (6 байт Акселерометр + 2 байти Температура + 6 байт Гіроскоп)
    HAL_I2C_Mem_Read(&i2cBus, MPU6050_ADDR, ACCEL_XOUT_H_REG, 1, Rec_Data, 14, 100);

    // Збирання старших та молодших байтів (16-бітні значення зі знаком)
    mpu6050Data.Accel_X_RAW = (int16_t)(Rec_Data[0] << 8 | Rec_Data[1]);
    mpu6050Data.Accel_Y_RAW = (int16_t)(Rec_Data[2] << 8 | Rec_Data[3]);
    mpu6050Data.Accel_Z_RAW = (int16_t)(Rec_Data[4] << 8 | Rec_Data[5]);

    mpu6050Data.Gyro_X_RAW = (int16_t)(Rec_Data[8] << 8 | Rec_Data[9]);
    mpu6050Data.Gyro_Y_RAW = (int16_t)(Rec_Data[10] << 8 | Rec_Data[11]);
    mpu6050Data.Gyro_Z_RAW = (int16_t)(Rec_Data[12] << 8 | Rec_Data[13]);

    mpu6050Data.Ax = ((int32_t)mpu6050Data.Accel_X_RAW * 100 + 8192) / 16384;
    mpu6050Data.Ay = ((int32_t)mpu6050Data.Accel_Y_RAW * 100 + 8192) / 16384;
    mpu6050Data.Az = ((int32_t)mpu6050Data.Accel_Z_RAW * 100 + 8192) / 16384;

    mpu6050Data.Gx = ((int32_t)mpu6050Data.Gyro_X_RAW * 100 + 65) / 131;
    mpu6050Data.Gy = ((int32_t)mpu6050Data.Gyro_Y_RAW * 100 + 65) / 131;
    mpu6050Data.Gz = ((int32_t)mpu6050Data.Gyro_Z_RAW * 100 + 65) / 131;

    return mpu6050Data;
}

