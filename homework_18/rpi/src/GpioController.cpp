#include "GpioController.hpp"
#include "common.hpp"
#include <gpiod.h>
#include <unistd.h>

GpioController::GpioController(std::string chipName, uint controlLine)
    : chipName(std::move(chipName)), controlLine(controlLine), request{}, chip{}
{
}

auto GpioController::init() -> void
{
    auto chipPath = std::string("/dev/") + chipName;

    chip = gpiod_chip_open(chipPath.c_str());

    if (chip == nullptr)
    {
        throw std::invalid_argument("Can't open GPIO chip");
    }

    LOG("Chip opened successfully!");

    // 2. Створюємо налаштування для ліній (Встановлюємо вихідний напрямок)
    auto *settings = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);
    // за бажанням можна задати початкове значення за замовчуванням (0):
    gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_INACTIVE);

    // 3. Додаємо лінії до конфігурації
    auto *line_cfg = gpiod_line_config_new();
    unsigned int offsets[] = {controlLine};

    // Додаємо конфігурацію для обох пінів одночасно
    gpiod_line_config_add_line_settings(line_cfg, offsets, 1, settings);

    // 4. Створюємо конфігурацію самого запиту (задаємо ім'я споживача "drone")
    auto *req_cfg = gpiod_request_config_new();
    gpiod_request_config_set_consumer(req_cfg, "drone");

    // 5. Запитуємо лінії у системи
    request = gpiod_chip_request_lines(chip, req_cfg, line_cfg);

    // Очищаємо тимчасові об'єкти конфігурації (вони більше не потрібні після створення request)
    gpiod_request_config_free(req_cfg);
    gpiod_line_config_free(line_cfg);
    gpiod_line_settings_free(settings);

    if (request == nullptr)
    {
        gpiod_chip_close(chip);

        throw std::invalid_argument("Can't reserve GPIO lines");
    }

    LOG("Lines reserved successfully!");
}

auto GpioController::setTemperatureControlLine(bool isTemperatureHigh) -> void
{
    gpiod_line_request_set_value(request, controlLine, isTemperatureHigh ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
}

// auto RpiCheckerGPIO::start() -> void
// {
//     gpiod_line_request_set_value(request, startLine, GPIOD_LINE_VALUE_ACTIVE);
//     usleep(80000);
//     gpiod_line_request_set_value(request, startLine, GPIOD_LINE_VALUE_INACTIVE);
// }

// auto RpiCheckerGPIO::drop() -> void
// {
//     gpiod_line_request_set_value(request, dropLine, GPIOD_LINE_VALUE_ACTIVE);
//     usleep(100000);
//     gpiod_line_request_set_value(request, dropLine, GPIOD_LINE_VALUE_INACTIVE);
// }

GpioController::~GpioController()
{
    gpiod_line_request_release(request);
    gpiod_chip_close(chip);
}
