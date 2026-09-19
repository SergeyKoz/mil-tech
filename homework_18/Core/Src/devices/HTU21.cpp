#include "devices/HTU21.h"
#include <math.h>

HTU21::HTU21(I2C_HandleTypeDef& i2cBus) : i2cBus(i2cBus) {}

auto HTU21::init() -> bool
{
    uint8_t cmd = HTU21D_CMD_RESET;
    HAL_StatusTypeDef status;

    // Відправляємо команду Soft Reset
    status = HAL_I2C_Master_Transmit(&i2cBus, HTU21D_I2C_ADDR, &cmd, 1, HAL_MAX_DELAY);
    HAL_Delay(15); // Час на перезавантаження датчика (~15 ms)

    return status == HAL_OK;   
}

auto HTU21::read() -> HTU21_t {
    int temp = 0.0f;
    int hum = 0.0f;

    if (readTemperature(&temp) == HAL_OK) {
        htu21Data.temperature = temp;
    }

    if (readHumidity(&hum) == HAL_OK) {
        htu21Data.humidity = hum;
    }

    return htu21Data;
}

HAL_StatusTypeDef HTU21::readTemperature(int *temperature) {
    uint8_t cmd = HTU21D_CMD_MEAS_TEMP;
    uint8_t data[3]; // [MSB, LSB, CRC]
    HAL_StatusTypeDef status;

    // 1. Відправляємо команду на початок вимірювання
    status = HAL_I2C_Master_Transmit(&i2cBus, HTU21D_I2C_ADDR, &cmd, 1, HAL_MAX_DELAY);
    
    if (status != HAL_OK) {
        return status;
    }

    // 2. Пауза для завершення вимірювання (для 14-біт розрядності ~50 мс)
    HAL_Delay(50);

    // 3. Зчитуємо 3 байти
    status = HAL_I2C_Master_Receive(&i2cBus, HTU21D_I2C_ADDR, data, 3, HAL_MAX_DELAY);
    
    if (status != HAL_OK) {
        return status;
    }

    // 4. Об'єднуємо байти (відкидаємо 2 молодших біти статусу: data[1] & 0xFC)
    uint16_t raw_temp = ((uint16_t)data[0] << 8) | (data[1] & 0xFC);

    // 5. Розрахунок у форматі x100 (ціле число):
    // Формула: Temp * 100 = -4685 + (17572 * raw_temp) / 65536
    // Додаємо 32768 (тобто 65536 / 2) для точного математичного округлення
    *temperature = -4685 + (int32_t)(((int64_t)17572 * raw_temp + 32768) >> 16);

    return HAL_OK;
}

HAL_StatusTypeDef HTU21::readHumidity(int *humidity) {
    uint8_t cmd = HTU21D_CMD_MEAS_HUMI;
    uint8_t data[3]; // [MSB, LSB, CRC]
    HAL_StatusTypeDef status;

    // 1. Відправляємо команду на початок вимірювання
    status = HAL_I2C_Master_Transmit(&i2cBus, HTU21D_I2C_ADDR, &cmd, 1, HAL_MAX_DELAY);
    
    if (status != HAL_OK) {
        return status;
    }

    // 2. Пауза для завершення вимірювання (для 12-біт розрядності ~16 мс)
    HAL_Delay(20);

    // 3. Зчитуємо 3 байти
    status = HAL_I2C_Master_Receive(&i2cBus, HTU21D_I2C_ADDR, data, 3, HAL_MAX_DELAY);
    
    if (status != HAL_OK) {
        return status;
    }

    // 4. Об'єднуємо байти (відкидаємо 2 молодших біти статусу: data[1] & 0xFC)
    uint16_t raw_humi = ((uint16_t)data[0] << 8) | (data[1] & 0xFC);

    // 5. Розрахунок у форматі x100 (ціле число):
    // Формула x100: RH * 100 = -600 + (12500 * raw_humi) / 65536
    // Додаємо 32768 (тобто 65536 / 2) для математичного округлення
    *humidity = -600 + (int32_t)(((int64_t)12500 * raw_humi + 32768) >> 16);

    // 6. Обмеження меж вологості від 0% до 100% (тобто 0..10000 у форматі x100)
    if (*humidity < 0) {
        *humidity = 0;
    } else if (*humidity > 10000) {
        *humidity = 10000;
    }

    return HAL_OK;
}