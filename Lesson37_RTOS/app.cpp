#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <cstdio>
#include <cstring>

extern I2C_HandleTypeDef  hi2c1;
extern UART_HandleTypeDef huart1;

namespace {

constexpr uint16_t kAdsAddr = 0x48 << 1;
constexpr uint8_t  kRegConv = 0x00;
constexpr uint8_t  kRegCfg  = 0x01;

constexpr uint16_t kCfgSingleA0 = 0xC383;

bool adsReadOnce(int16_t& raw) {
    uint8_t cfg[2] = { uint8_t(kCfgSingleA0 >> 8), uint8_t(kCfgSingleA0) };
    if (HAL_I2C_Mem_Write(&hi2c1, kAdsAddr, kRegCfg, I2C_MEMADD_SIZE_8BIT,
                          cfg, 2, 100) != HAL_OK) return false;
    vTaskDelay(pdMS_TO_TICKS(10));
    uint8_t d[2];
    if (HAL_I2C_Mem_Read(&hi2c1, kAdsAddr, kRegConv, I2C_MEMADD_SIZE_8BIT,
                         d, 2, 100) != HAL_OK) return false;
    raw = int16_t((d[0] << 8) | d[1]);
    return true;
}

struct Sample { uint32_t t_ms; int16_t raw; };

QueueHandle_t q;
TaskHandle_t  btnTaskH;
volatile bool txEnabled = true;

SemaphoreHandle_t uartMtx;

void uartSend(const char* s, size_t n) {
    if (xSemaphoreTake(uartMtx, pdMS_TO_TICKS(100)) == pdTRUE) {
        HAL_UART_Transmit(&huart1, (uint8_t*)s, n, 100);
        xSemaphoreGive(uartMtx);

        // toogle LED on PC13 to indicate UART activity
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    }
}

void heartbeatTask(void*) {
    TickType_t wake = xTaskGetTickCount();
    uint32_t n = 0;
    char line[40];
    for (;;) {
        vTaskDelayUntil(&wake, pdMS_TO_TICKS(1000));
        const int len = snprintf(line, sizeof line, "hb #%lu t=%lu ms\r\n",
                                 (unsigned long)++n,
                                 (unsigned long)(xTaskGetTickCount() * portTICK_PERIOD_MS));
        uartSend(line, len);
    }
}

void sensorTask(void*) {
    TickType_t wake = xTaskGetTickCount();
    for (;;) {
        vTaskDelayUntil(&wake, pdMS_TO_TICKS(100));
        Sample s;
        s.t_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
        if (adsReadOnce(s.raw))
            xQueueSend(q, &s, 0);
    }
}

void commsTask(void*) {
    Sample s;
    char line[48];
    for (;;) {
        xQueueReceive(q, &s, portMAX_DELAY);
        if (!txEnabled) continue;
        const int32_t mv = int32_t(s.raw) * 125 / 1000;
        const int n = snprintf(line, sizeof line,
                               "t=%lu ms  raw=%d  U=%ld mV\r\n",
                               (unsigned long)s.t_ms, s.raw, (long)mv);
        uartSend(line, n);
    }
}

void buttonTask(void*) {
    TickType_t last = 0;
    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        const TickType_t now = xTaskGetTickCount();
        if (now - last < pdMS_TO_TICKS(50)) continue;
        last = now;
        txEnabled = !txEnabled;
        const char* msg = txEnabled ? "tx: ON\r\n" : "tx: OFF\r\n";
       // HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        uartSend(msg, strlen(msg));
    }
}

}

extern "C" void app_start(void) {
    q = xQueueCreate(8, sizeof(Sample));
    uartMtx = xSemaphoreCreateMutex();
    xTaskCreate(sensorTask, "sensor", 256, nullptr, 4, nullptr);
    xTaskCreate(commsTask,  "comms",  256, nullptr, 3, nullptr);
    xTaskCreate(buttonTask, "button", 128, nullptr, 2, &btnTaskH);
    xTaskCreate(heartbeatTask, "hb", 192, nullptr, 1, nullptr);
}

extern "C" void HAL_GPIO_EXTI_Callback(uint16_t pin) {
    if (pin == GPIO_PIN_0) {
        BaseType_t woken = pdFALSE;
        vTaskNotifyGiveFromISR(btnTaskH, &woken);
        portYIELD_FROM_ISR(woken);
    }
}
