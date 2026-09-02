cmake_minimum_required(VERSION 3.20)

# Окремий CMake-проєкт для домашньої роботи 04. Його можна збирати як через root CMakeLists.txt, так
# і напряму з цієї директорії.
project(stm32_server_app CXX)

# Код домашньої роботи збирається з тим самим стандартом, що й лекційні приклади.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
    nlohmann_json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
                      DOWNLOAD_EXTRACT_TIMESTAMP ON)

add_executable(stm32_server_app src/main.cpp)

target_sources(stm32_server_app PRIVATE 
    src/IntervalWorker.cpp 
    src/UartReader.cpp 
    src/GpioController.cpp
    src/SensorController.cpp
    src/parser/JsonTelemetryParser.cpp
    )

target_include_directories(stm32_server_app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(stm32_server_app PRIVATE nlohmann_json::nlohmann_json gpiod)

target_compile_options(stm32_server_app PRIVATE -Wall -Wextra -pedantic -Werror)

# include(GoogleTest) gtest_discover_tests(ballistics_tests)
