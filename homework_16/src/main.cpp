#include <iostream>
#include "CliParams.hpp"
#include "I2CBus.hpp"
#include "I2CDeviceListener.hpp"
#include "devices/ASD1115Device.hpp"
#include "devices/MPU5060Device.hpp"

auto main(int argc, char* argv[]) -> int
{
    // ADS1115 (0x48) і MPU-6050 (0x68)
    // LD_PRELOAD=/home/dev/mil-tech/homework_16/lib/libi2csim.so

    try {
        auto params = CliParams::parse(argc, argv);

        auto i2cBus = std::make_shared<I2CBus>(params.source);
        i2cBus->open();

        auto asd1115 = std::make_shared<ASD1115Device>();
        asd1115->connect(i2cBus);

        auto mpu5060 = std::make_shared<MPU5060Device>();
        mpu5060->connect(i2cBus);

        auto deviceListener = std::make_unique<I2CDeviceListener>(std::chrono::milliseconds(params.interval));
        deviceListener->addDevice(asd1115);
        deviceListener->addDevice(mpu5060);
        deviceListener->start();

        while (!deviceListener->isThreadReady()) {
            std::this_thread::yield();
        }

        deviceListener->wait();
    }
    catch (const std::runtime_error& ex) {
        std::cerr << ex.what() << '\n';

        return 1;
    }
    catch (const std::invalid_argument& ex) {
        std::cerr << ex.what() << '\n';

        return 1;
    }

    return 0;
}