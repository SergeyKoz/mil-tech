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

    # MAVROS параметри
    fcu_url = LaunchConfiguration("fcu_url")
    gcs_url = LaunchConfiguration("gcs_url")
    tgt_system = LaunchConfiguration("tgt_system")
    tgt_component = LaunchConfiguration("tgt_component")
    


    scenario_path = PathJoinSubstitution(
        [FindPackageShare("ballistics_simulator"), "config", scenario]
    )

    ballistic_table_path = PathJoinSubstitution(
        [FindPackageShare("ballistics_simulator"), "config", "ballistic_table.txt"]
    )

    # Нода MAVROS
    mavros_node = Node(
        package="mavros",
        executable="mavros_node",
        name="mavros_node", # _qgc_node
        # namespace="qgc",
        output="screen",
        parameters=[
            {
                "fcu_url": fcu_url,
                "gcs_url": gcs_url,
                "target_system_id": 1, # ParameterValue(tgt_system, value_type=int),
                "target_component_id": 1, #  arameterValue(tgt_component, value_type=int),
                "fcu_protocol": "v2.0",
                "plugin_allowlist": [
                    "sys_status",
                    "sys_version",
                    "sys_time",
                    "command",
                    "local_position",
                    "global_position",
                    "imu",
                    "param",
                    "rc_io",
                    # "setpoint_position",
                    # "setpoint_velocity",
                    # "setpoint_raw",
                    "mission",
                    # "waypoint",
                    "manual_control",
                    # "state",
                    "timesync"
                ]
                # "plugin_denylist": [
                #     # "companion_process_status",
                #     # "adsb",
                #     # "camera",
                #     # "cam_imu_sync",
                #     # "cellular_status",
                #     # "actuator_control"
                # ]
            }
        ],
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

            DeclareLaunchArgument(
                "fcu_url",
                default_value="udp://:14551@",
                description="FCU connection URL",
            ),

            # gcs_url: IP-адреса вашого ПК з QGroundControl та порт 14550 (за замовчуванням у QGC)
            DeclareLaunchArgument(
                "gcs_url",
                default_value="udp://@192.168.1.103:14550",  # Вкажіть тут IP вашого комп'ютера з QGC
                description="GCS connection URL (QGroundControl)",
            ),
            DeclareLaunchArgument(
                "tgt_system",
                default_value="1",
                description="Target system ID",
            ),
            DeclareLaunchArgument(
                "tgt_component",
                default_value="1",
                description="Target component ID",
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
            Node(
                package="ballistics_simulator",
                executable="autopilot_node",
                parameters=[
                    {
                        "ballistic_table_path": ballistic_table_path,
                    }
                ],
            ),
            # mavros_node,
        ]
    )
