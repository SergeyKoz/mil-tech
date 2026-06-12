cmake_minimum_required(VERSION 3.20)

# Окремий CMake-проєкт для домашньої роботи 04. Його можна збирати як через root CMakeLists.txt, так
# і напряму з цієї директорії.
project(telemetry_check CXX)

# Код домашньої роботи збирається з тим самим стандартом, що й лекційні приклади.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Один executable для задачі з UGV odometry.
add_executable(ballistics_cli src/main.cpp)

add_library(ballistics src/ballistics.cpp)

target_include_directories(ballistics PUBLIC include)
target_link_libraries(ballistics_cli PRIVATE ballistics)

# Базові compiler warnings допомагають побачити підозрілі місця до runtime.
target_compile_options(ballistics_cli PRIVATE -Wall -Wextra -pedantic -Werror)
target_compile_options(ballistics PRIVATE -Wall -Wextra -pedantic -Werror)

add_executable(ballistics_tests tests/ballistics_tests.cpp)
target_link_libraries(ballistics_tests ballistics GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(ballistics_tests)
