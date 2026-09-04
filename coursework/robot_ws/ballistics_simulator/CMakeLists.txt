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

# Власні msg/srv описують контракт між world node і керуючою системою.
find_package(rosidl_default_generators REQUIRED)
rosidl_generate_interfaces(
    ${PROJECT_NAME}
    "msg/CellObservation.msg"
    "msg/LocalScan.msg"
    "msg/MoveCommand.msg"
    "msg/EnemyDown.msg"
    "msg/RobotMetrics.msg"
    "msg/RobotResult.msg"
    "msg/StudentStatus.msg"
    "srv/PayloadTrigger.srv")

ament_auto_add_library(underground_world_core src/scenario_loader.cpp src/world_model.cpp
                       src/world_explorer.cpp src/checker_uart_listener.cpp)
target_include_directories(underground_world_core PUBLIC include)

ament_auto_add_executable(underground_world_node src/underground_world_node.cpp)
target_link_libraries(underground_world_node underground_world_core)

rosidl_get_typesupport_target(cpp_typesupport_target ${PROJECT_NAME} "rosidl_typesupport_cpp")
target_link_libraries(underground_world_node "${cpp_typesupport_target}")

ament_auto_add_executable(world_explorer_node src/world_explorer_node.cpp)
target_link_libraries(world_explorer_node underground_world_core)

rosidl_get_typesupport_target(cpp_typesupport_target ${PROJECT_NAME} "rosidl_typesupport_cpp")
target_link_libraries(world_explorer_node "${cpp_typesupport_target}")

ament_auto_add_executable(trigger_service_node src/trigger_service_node.cpp)
target_link_libraries(trigger_service_node underground_world_core)

rosidl_get_typesupport_target(cpp_typesupport_target ${PROJECT_NAME} "rosidl_typesupport_cpp")
target_link_libraries(trigger_service_node "${cpp_typesupport_target}")

ament_auto_add_executable(checker_data_provider_node src/checker_data_provider_node.cpp)
target_link_libraries(checker_data_provider_node underground_world_core)

rosidl_get_typesupport_target(cpp_typesupport_target ${PROJECT_NAME} "rosidl_typesupport_cpp")
target_link_libraries(checker_data_provider_node "${cpp_typesupport_target}")

if(BUILD_TESTING)
    find_package(ament_cmake_gtest REQUIRED)

    ament_add_gtest(provided_world_contract_test test/provided_world_contract_test.cpp)
    target_include_directories(provided_world_contract_test PRIVATE include)
    target_link_libraries(provided_world_contract_test underground_world_core)
    target_compile_definitions(provided_world_contract_test
                               PRIVATE UNDERGROUND_WORLD_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

    ament_add_gtest(world_explorer_test test/world_explorer_test.cpp)
    target_include_directories(world_explorer_test PRIVATE include)
    target_link_libraries(world_explorer_test underground_world_core)
    target_compile_definitions(world_explorer_test
                               PRIVATE UNDERGROUND_WORLD_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
endif()

ament_export_dependencies(rosidl_default_runtime)

# launch/ і config/ встановлюються у share, щоб ros2 launch знаходив сценарії.
ament_auto_package(USE_SCOPED_HEADER_INSTALL_DIR INSTALL_TO_SHARE launch config)
