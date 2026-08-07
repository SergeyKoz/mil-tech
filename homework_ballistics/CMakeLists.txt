cmake_minimum_required(VERSION 3.20)

# Окремий CMake-проєкт для домашньої роботи 04. Його можна збирати як через root CMakeLists.txt, так
# і напряму з цієї директорії.
project(ballistics CXX)

# Код домашньої роботи збирається з тим самим стандартом, що й лекційні приклади.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenSSL REQUIRED)

include(FetchContent)
FetchContent_Declare(
    nlohmann_json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
                      DOWNLOAD_EXTRACT_TIMESTAMP ON)

FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.18.3)

FetchContent_MakeAvailable(nlohmann_json httplib)

# Один executable для задачі з UGV odometry.
add_executable(ballistics_app src/main.cpp)

target_sources(
    ballistics_app
    PRIVATE src/MissionFactory.cpp
            src/IntervalWorker.cpp
            src/ThreadDronePhysics.cpp
            src/ThreadMissionProcessor.cpp
            src/Target.cpp
            src/providers/JsonTargetProvider.cpp
            src/providers/ThreadTargetProvider.cpp
            src/providers/JsonAmmoProvider.cpp
            src/providers/CheckerTargetProvider.cpp
            src/solvers/AnalyticalSolver.cpp
            src/solvers/BallisticTable.cpp
            src/solvers/TableSolver.cpp
            src/config/JsonConfigLoader.cpp
            src/config/CheckerConfigLoader.cpp
            src/config/AppConfigLoader.cpp
            src/export/JsonSimulationExport.cpp
            src/states/StoppedState.cpp
            src/states/MovingState.cpp
            src/states/TurningState.cpp
            src/states/AcceleratingState.cpp
            src/states/DeceleratingState.cpp
            src/TargetSelector.cpp
            src/TestFilesRepository.cpp
            src/TestFilesStorage.cpp)

target_include_directories(ballistics_app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(ballistics_app PRIVATE nlohmann_json::nlohmann_json httplib::httplib
                                             OpenSSL::SSL OpenSSL::Crypto)
target_compile_definitions(ballistics_app PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)
# Базові compiler warnings допомагають побачити підозрілі місця до runtime.
# target_compile_options(ballistics_cli PRIVATE -Wall -Wextra -pedantic -Werror)
target_compile_options(ballistics_app PRIVATE -Wall -Wextra -pedantic -Werror)

# include(GoogleTest) gtest_discover_tests(ballistics_tests)

add_executable(rpi_simulation_app src/rpi.cpp)

target_sources(
    rpi_simulation_app
    PRIVATE src/MissionFactory.cpp
            src/IntervalWorker.cpp
            src/ThreadDronePhysics.cpp
            src/DroneAutopilot.cpp
            src/Target.cpp
            src/providers/JsonTargetProvider.cpp
            src/providers/ThreadTargetProvider.cpp
            src/providers/JsonAmmoProvider.cpp
            src/providers/CheckerTargetProvider.cpp
            src/solvers/AnalyticalSolver.cpp
            src/solvers/BallisticTable.cpp
            src/solvers/TableSolver.cpp
            src/config/JsonConfigLoader.cpp
            src/config/CheckerConfigLoader.cpp
            src/export/JsonSimulationExport.cpp
            src/states/StoppedState.cpp
            src/states/MovingState.cpp
            src/states/TurningState.cpp
            src/states/AcceleratingState.cpp
            src/states/DeceleratingState.cpp
            src/checkerRpi/CheckerRpiGPIO.cpp
            src/checkerRpi/CheckerRpiUART.cpp
            src/TargetSelector.cpp)

target_include_directories(rpi_simulation_app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(rpi_simulation_app PRIVATE nlohmann_json::nlohmann_json gpiod)
target_compile_options(rpi_simulation_app PRIVATE -Wall -Wextra -pedantic -Werror)
