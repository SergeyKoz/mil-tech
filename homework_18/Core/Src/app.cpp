#include "app.h"

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
// #include <stdio.h>

extern "C"
{
    void app_start() {
        HAL_Delay(1000);
    }
}