cmake_minimum_required(VERSION 3.20)
project(ballistics_simulator)

# C++20 - базовий стандарт курсу.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ament_auto бере залежності з package.xml і тримає CMake коротшим.
find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

find_package(PkgConfig REQUIRED)
set(ENV{PKG_CONFIG_PATH} "/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig:$ENV{PKG_CONFIG_PATH}")
pkg_check_modules(GPIOD REQUIRED IMPORTED_TARGET libgpiod>=2.0)

# Власні msg/srv описують контракт між world node і керуючою системою.
find_package(rosidl_default_generators REQUIRED)
rosidl_generate_interfaces(
    ${PROJECT_NAME}
    "msg/DroneConfig.msg"
    "msg/AmmoConfig.msg"
    "msg/Telemetry.msg"
    "msg/Target.msg"
    "msg/ControlCommand.msg"
    "msg/StartCommand.msg"
    "msg/DropCommand.msg"
    "srv/DropTrigger.srv"
    "srv/StartTrigger.srv")

ament_auto_add_library(ballistics_simulator_core 
                       src/checker_uart_listener.cpp
                       src/checker_gpio_controller.cpp
                       src/autopilot.cpp
                       src/solvers/table_solver.cpp
                       src/solvers/ballistic_table.cpp
                       src/providers/targets_provider.cpp
                       src/target_selector.cpp
                       src/states/accelerating_state.cpp
                       src/states/decelerating_state.cpp
                       src/states/moving_state.cpp
                       src/states/stopped_state.cpp
                       src/states/turning_state.cpp)
target_include_directories(ballistics_simulator_core PUBLIC include ${GPIOD_INCLUDE_DIRS})
#${GPIOD_LIBRARIES}
target_link_libraries(ballistics_simulator_core PkgConfig::GPIOD)

rosidl_get_typesupport_target(cpp_typesupport_target ${PROJECT_NAME} "rosidl_typesupport_cpp")

ament_auto_add_executable(checker_data_provider_node src/checker_data_provider_node.cpp)
target_link_libraries(checker_data_provider_node ballistics_simulator_core)
target_link_libraries(checker_data_provider_node "${cpp_typesupport_target}")

ament_auto_add_executable(autopilot_node src/autopilot_node.cpp)
target_link_libraries(autopilot_node ballistics_simulator_core)
target_link_libraries(autopilot_node "${cpp_typesupport_target}")

ament_auto_add_executable(drop_service_node src/drop_service_node.cpp)
target_link_libraries(drop_service_node ballistics_simulator_core)
target_link_libraries(drop_service_node "${cpp_typesupport_target}")

ament_auto_add_executable(start_service_node src/start_service_node.cpp)
target_link_libraries(start_service_node ballistics_simulator_core)
target_link_libraries(start_service_node "${cpp_typesupport_target}")

ament_auto_add_executable(qgc_bridge_node src/qgc_bridge_node.cpp)
target_link_libraries(qgc_bridge_node ballistics_simulator_core)
target_link_libraries(qgc_bridge_node "${cpp_typesupport_target}")

if(BUILD_TESTING)
    find_package(ament_cmake_gtest REQUIRED)

    # ament_add_gtest(provided_world_contract_test test/provided_world_contract_test.cpp)
    # target_include_directories(provided_world_contract_test PRIVATE include)
    # target_link_libraries(provided_world_contract_test underground_world_core)
    # target_compile_definitions(provided_world_contract_test
    #                            PRIVATE UNDERGROUND_WORLD_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

    ament_add_gtest(autopilot_test test/autopilot_test.cpp)
    target_include_directories(autopilot_test PRIVATE include)
    target_link_libraries(autopilot_test ballistics_simulator_core)

    ament_add_gtest(accelerating_state_test test/states/accelerating_state_test.cpp)
    target_include_directories(accelerating_state_test PRIVATE include)
    target_link_libraries(accelerating_state_test ballistics_simulator_core)

    ament_add_gtest(target_selector_test test/target_selector_test.cpp)
    target_include_directories(target_selector_test PRIVATE include)
    target_link_libraries(target_selector_test ballistics_simulator_core)
endif()

ament_export_dependencies(rosidl_default_runtime)

# launch/ і config/ встановлюються у share, щоб ros2 launch знаходив сценарії.
ament_auto_package(USE_SCOPED_HEADER_INSTALL_DIR INSTALL_TO_SHARE launch config)
