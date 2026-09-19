#include "devices/BMP280.h"
#include <math.h>

BMP280::BMP280(SPI_HandleTypeDef& spiBus) : spiBus(spiBus) {}

auto BMP280::init() -> bool
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PinState::GPIO_PIN_RESET);
    HAL_Delay(50);

    uint8_t raw_id = 0;
    readRegs(0xD0, &raw_id, 1);

    uint8_t chip_id = raw_id >> 1; 

    if (chip_id != 0x58 && raw_id != 0x58) {
        return false; // Датчик не знайдено або помилка SPI
    }

    // 2. Налаштування режимів (Регістр ctrl_meas 0xF4)
    // osrs_t x1 (001), osrs_p x1 (001), Normal mode (11) -> 0x27
    writeReg(0xF4, 0x27);

    // 3. Конфігурація (Регістр config 0xF5)
    // t_sb 0.5ms (000), filter off (000), spi3w_en off (0) -> 0x00
    writeReg(0xF5, 0x00);

    readTrimmingParameters();

    return true;   
}

auto BMP280::read() -> BMP280_t {
    int32_t raw_temp = 0;
    int32_t raw_press = 0;

    readRawData(&raw_temp, &raw_press);

    int32_t temp_centidegrees = compensateT(raw_temp);
    bmp280Data.temperature = temp_centidegrees; // e.g. 2453 -> 24.53 °C

    uint32_t press_pa = compensateP(raw_press);
    bmp280Data.pressure_pa = press_pa;
    bmp280Data.pressure_hpa = (press_pa + 50) / 100; // Перевід в гектопаскалі (hPa)

    // 3. Обчислюємо висоту над рівнем моря в метрах
    bmp280Data.altitude_m = calculateAltitude(bmp280Data.pressure_pa);

    return bmp280Data;
}

void BMP280::writeReg(uint8_t reg, uint8_t value) {
    reg &= ~0x80; // Біт 7 = 0 для запису
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); // CS -> LOW
    HAL_SPI_Transmit(&spiBus, &reg, 1, HAL_MAX_DELAY);
    HAL_SPI_Transmit(&spiBus, &value, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);   // CS -> HIGH
}

void BMP280::readRegs(uint8_t reg, uint8_t *pData, uint16_t length) {
    reg |= 0x80; // Біт 7 = 1 для читання
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); // CS -> LOW
    HAL_SPI_Transmit(&spiBus, &reg, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&spiBus, pData, length, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);   // CS -> HIGH
}

// Функція зчитування сирих даних (6 байт: 3 на тиск, 3 на температуру)
void BMP280::readRawData(int32_t *raw_temp, int32_t *raw_press) {
    uint8_t data[6];
    readRegs(0xF7, data, 6);

    *raw_press = (int32_t)(((uint32_t)data[0] << 12) | ((uint32_t)data[1] << 4) | ((uint32_t)data[2] >> 4));
    *raw_temp  = (int32_t)(((uint32_t)data[3] << 12) | ((uint32_t)data[4] << 4) | ((uint32_t)data[5] >> 4));
}

void BMP280::readTrimmingParameters() {
    uint8_t calib[24];
    // Remember to set bit 7 for SPI read: 0x88 | 0x80 = 0x08
    readRegs(0x88, calib, 24); 

    dig_T1 = (uint16_t)(calib[1] << 8 | calib[0]);
    dig_T2 = (int16_t)(calib[3] << 8 | calib[2]);
    dig_T3 = (int16_t)(calib[5] << 8 | calib[4]);

    dig_P1 = (uint16_t)(calib[7] << 8 | calib[6]);
    dig_P2 = (int16_t)(calib[9] << 8 | calib[8]);
    dig_P3 = (int16_t)(calib[11] << 8 | calib[10]);
    dig_P4 = (int16_t)(calib[13] << 8 | calib[12]);
    dig_P5 = (int16_t)(calib[15] << 8 | calib[14]);
    dig_P6 = (int16_t)(calib[17] << 8 | calib[16]);
    dig_P7 = (int16_t)(calib[19] << 8 | calib[18]);
    dig_P8 = (int16_t)(calib[21] << 8 | calib[20]);
    dig_P9 = (int16_t)(calib[23] << 8 | calib[22]);
}


// Returns temperature in degC, resolution is 0.01 degC. 
// Output value of "5123" equals 51.23 degC.
int32_t BMP280::compensateT(int32_t adc_T) {
    int32_t var1, var2, T;
    
    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    
    return T;
}

// Компенсація тиску (повертає значення в Паскалях в форматі Q24.8 або uint32_t в Pa)
uint32_t BMP280::compensateP(int32_t adc_P) {
    int64_t var1, var2, p;

    // Увага: BMP280_Compensate_T() має бути викликана БОДІ-ЯК ПЕРЕД цією функцією,
    // оскільки вона формує глобальну змінну t_fine!
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;

    if (var1 == 0) {
        return 0; // Запобігання діленню на нуль
    }

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;

    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);

    return (uint32_t)(p >> 8); // Повертає тиск у Pa (Паскалі)
}

int BMP280::calculateAltitude(int pressure_pa) {

    // 1. Різниця тиску від рівня моря
    int32_t diff = SEALEVELPRESSURE_PA - pressure_pa;

    // 2. Лінійне наближення близько рівня моря: ~1 hPa (100 Pa) ≈ 8.43m
    // Використовуємо масштабування на 1000 для точності цілочисельного ділення:
    // (diff * 843) / 10000 -> дає висоту в метрах
    int32_t altitude_m = (diff * 843) / 10000;

    // 3. Квадратична поправка для більших висот (компенсація розрядження повітря)
    if (altitude_m > 100) {
        altitude_m += (altitude_m * altitude_m) / 15000;
    }

    return altitude_m;

    // return 44330.0f * (1.0f - powf(pressure_hpa / SEALEVELPRESSURE_HPA, 0.19029495f));
}