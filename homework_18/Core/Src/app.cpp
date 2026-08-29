#include "app.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "cmsis_gcc.h"
#include "devices/MPU6050.h"
#include "devices/BMP280.h"
#include "devices/HTU21.h"
#include "stm32f4xx_hal_uart.h"
#include <stdio.h>

extern TIM_HandleTypeDef htim1;
extern I2C_HandleTypeDef hi2c1;
extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart1;

volatile bool tick = false;

MPU6050 mpu6050(hi2c1);
BMP280 bmp280(hspi1);
HTU21 htu21(hi2c1);

void uartSend(const char* s, size_t n){
    HAL_UART_Transmit(&huart1, (uint8_t*)s, n, 100);
}

extern "C" void app_start() {
    HAL_TIM_Base_Start_IT(&htim1);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PinState::GPIO_PIN_SET);

    auto isMpu5060Ready = mpu6050.init();
    auto isBmp280Ready = bmp280.init();
    auto isHtu21Ready = htu21.init(); 

    char buffer[128];

    while (1) {
        if (tick) {
            tick = false;
            
            if (isMpu5060Ready) {
                auto mpu6050data = mpu6050.read();
                auto len = mpu6050data.toJson(buffer, sizeof(buffer));
                uartSend(buffer, len);
            }

            if (isBmp280Ready) {
                auto bmp280data = bmp280.read();
                auto len = bmp280data.toJson(buffer, sizeof(buffer));
                uartSend(buffer, len);
            }

            if (isHtu21Ready) {                
                auto htu21data =  htu21.read();
                auto len = htu21data.toJson(buffer, sizeof(buffer));
                uartSend(buffer, len);
            }

            GPIO_PinState pin03State = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3);
            GPIO_PinState pin13State = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_13);

            if (pin03State != pin13State) {
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, pin03State);
            }        

            __WFI();
            
        }       
    }
}

extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* t) {
    if (t->Instance == TIM1) {
        tick = true;
    }
}
