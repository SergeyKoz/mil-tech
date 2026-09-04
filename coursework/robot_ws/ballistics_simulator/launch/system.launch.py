from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    # Сценарій можна змінити без редагування launch-файлу:
    # ros2 launch ballistics_simulator system.launch.py scenario:=small_rooms.yaml
    scenario = LaunchConfiguration("scenario")
    move_commit_period_ms = LaunchConfiguration("move_commit_period_ms")
    scenario_path = PathJoinSubstitution(
        [FindPackageShare("ballistics_simulator"), "config", scenario]
    )

    # world_node = Node(
    #     package="ballistics_simulator",
    #     executable="underground_world_node",
    #     name="underground_world_node",
    #     output="screen",
    #     parameters=[
    #         {
    #             "scenario_path": scenario_path,
    #             "move_commit_period_ms": ParameterValue(
    #                 move_commit_period_ms, value_type=int
    #             ),
    #         }
    #     ],
    # )

    # Тут можна додати керуючі ноди або інший launch-файл з рішенням.
    return LaunchDescription(
        [
            DeclareLaunchArgument(
                "scenario",
                default_value="training_corridor.yaml",
                description="Scenario YAML file from ballistics_simulator/config",
            ),
            DeclareLaunchArgument(
                "move_commit_period_ms",
                default_value="50",
                description="Delay before applying queued move commands",
            ),
            # world_node,
            # Node(
            #     package="ballistics_simulator",
            #     executable="world_explorer_node",
            #     parameters=[
            #         {
            #             "scenario_path": scenario_path,
            #         }
            #     ],
            #     # parameters=[config],
            #     # arguments=["--ros-args", "--log-level", log_level],
            # ),
            # Node(
            #     package="ballistics_simulator",
            #     executable="trigger_service_node",
            #     parameters=[],
            #     # parameters=[config],
            #     # arguments=["--ros-args", "--log-level", log_level],
            # ),
            Node(
                package="ballistics_simulator",
                executable="checker_data_provider_node",
                parameters=[],
            ),
        ]
    )
