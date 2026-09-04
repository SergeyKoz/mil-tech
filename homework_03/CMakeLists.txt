cmake_minimum_required(VERSION 3.20)

# Окремий CMake-проєкт для домашньої роботи 04. Його можна збирати як через root CMakeLists.txt, так
# і напряму з цієї директорії.
project(simulation CXX)

# Код домашньої роботи збирається з тим самим стандартом, що й лекційні приклади.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
    nlohmann_json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
                      DOWNLOAD_EXTRACT_TIMESTAMP ON)
FetchContent_MakeAvailable(nlohmann_json)

# Один executable для задачі з UGV odometry.
add_executable(simulation src/main.cpp)

# Базові compiler warnings допомагають побачити підозрілі місця до runtime.
target_compile_options(simulation PRIVATE -Wall -Wextra -pedantic)

target_link_libraries(simulation PRIVATE nlohmann_json::nlohmann_json)
