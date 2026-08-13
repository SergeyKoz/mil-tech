cmake_minimum_required(VERSION 3.20)

# Окремий CMake-проєкт для домашньої роботи 04. Його можна збирати як через root CMakeLists.txt, так
# і напряму з цієї директорії.
project(i2c_app CXX)

# Код домашньої роботи збирається з тим самим стандартом, що й лекційні приклади.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(i2c_app src/main.cpp)

target_sources(i2c_app PRIVATE src/IntervalWorker.cpp src/I2CDeviceListener.cpp src/I2CBus.cpp
                               src/devices/ASD1115Device.cpp src/devices/MPU5060Device.cpp)

target_include_directories(i2c_app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_options(i2c_app PRIVATE -Wall -Wextra -pedantic -Werror)

# include(GoogleTest) gtest_discover_tests(ballistics_tests)
