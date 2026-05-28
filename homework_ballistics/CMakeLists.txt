cmake_minimum_required(VERSION 3.20)

# Окремий CMake-проєкт для домашньої роботи 04. Його можна збирати як через root CMakeLists.txt, так
# і напряму з цієї директорії.
project(ballistics CXX)

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
add_executable(ballistics_app src/main.cpp)

target_sources(
    ballistics_app
    PRIVATE src/MissionFactory.cpp
            src/MissionProcessor.cpp
            src/providers/JsonTargetProvider.cpp
            src/solvers/AnalyticalSolver.cpp
            src/config/JsonConfigLoader.cpp
            src/export/JsonSimulationExport.cpp
            src/providers/AmmoProvider.cpp
            src/TargetSelector.cpp)

target_include_directories(ballistics_app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(ballistics_app PRIVATE nlohmann_json::nlohmann_json)

# Базові compiler warnings допомагають побачити підозрілі місця до runtime.
# target_compile_options(ballistics_cli PRIVATE -Wall -Wextra -pedantic -Werror)
target_compile_options(ballistics_app PRIVATE -Wall -Wextra -pedantic -Werror)

# include(GoogleTest) gtest_discover_tests(ballistics_tests)
