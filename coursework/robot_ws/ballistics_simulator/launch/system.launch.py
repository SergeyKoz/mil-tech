from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch_ros.substitutions import FindPackageShare

def generate_launch_description():
    parameters = PathJoinSubstitution([
        FindPackageShare("ballistics_simulator"),
        "config",
        LaunchConfiguration("parameters"),
    ])

    # MAVROS параметри
    fcu_url = LaunchConfiguration("fcu_url")
    gcs_url = LaunchConfiguration("gcs_url")
    tgt_system = LaunchConfiguration("tgt_system")
    tgt_component = LaunchConfiguration("tgt_component")
    
    ballistic_table_path = PathJoinSubstitution(
        [FindPackageShare("ballistics_simulator"), "config", "ballistic_table.txt"]
    )

    # Нода MAVROS
    mavros_node = Node(
        package="mavros",
        executable="mavros_node",
        output="screen",
        parameters=[
            {
                "fcu_url": fcu_url,
                "gcs_url": gcs_url,
                "target_system_id": ParameterValue(tgt_system, value_type=int),
                "target_component_id": ParameterValue(tgt_component, value_type=int),
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
                    "state",
                    # "rc_io",
                    "setpoint_position",
                    # "setpoint_velocity",
                    "setpoint_raw",
                    "frame_transforms",
                    # "mission",
                    # "waypoint",
                    "manual_control",
                    "timesync",
                    "vision_pose",
                    "gcs_bridge",
                    "adsb",
                    "gps_input"
                    # "hil"
                ]
                # "plugin_denylist": [              
                # ]
            }
        ],
    )

    # Тут можна додати керуючі ноди або інший launch-файл з рішенням.
    return LaunchDescription(
        [
            DeclareLaunchArgument("parameters", default_value="parameters.yaml"),
            DeclareLaunchArgument(
                "fcu_url",
                default_value="udp://:14551@",
                description="FCU connection URL",
            ),
            # gcs_url: IP-адреса вашого ПК з QGroundControl та порт 14550 (за замовчуванням у QGC)
            DeclareLaunchArgument(
                "gcs_url",
                # default_value="udp://@192.168.173.129:14550", # work 192.168.173.124
                default_value="udp://@192.168.0.103:14550",  # home
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
            DeclareLaunchArgument(
                "tgt_component",
                default_value="1",
                description="Target component ID",
            ),
            Node(
                package="ballistics_simulator",
                executable="checker_data_provider_node",
                parameters=[parameters],
                # arguments=["--ros-args", "--log-level", log_level],
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
            Node(
                package="ballistics_simulator",
                executable="drop_service_node",
                parameters=[], 
            ),
            Node(
                package="ballistics_simulator",
                executable="start_service_node",
                parameters=[],
            ),
            Node(
                package="ballistics_simulator",
                executable="start_service_node",
                parameters=[],
            ),
            mavros_node,
            Node(
                package="ballistics_simulator",
                executable="qgc_bridge_node",
                output="screen",
                parameters=[parameters],
            ),
        ]
    )
