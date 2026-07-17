SHELL := /bin/bash

AntiDroneTurretApp/build:
	#. /opt/ros/jazzy/setup.bash
	# && \
	cd ./homework_13/robot_ws && \
	pwd && \
	colcon build --symlink-install --packages-select antidrone_turret

AntiDroneTurretApp/launch: AntiDroneTurretApp/build
	cd ./homework_13/robot_ws && \
	source install/setup.bash && \
	ros2 launch antidrone_turret system.launch.py track:=reload_pressure.csv
	# ros2 launch antidrone_turret system.launch.py track:=low_confidence_no_trigger.csv
	# ros2 launch antidrone_turret system.launch.py track:=far_flyby_no_trigger.csv
	# ros2 launch antidrone_turret system.launch.py track:=approach_trigger.csv

AntiDroneTurretApp/build-ci:
	#. /opt/ros/jazzy/setup.bash
	# && \
	cd ./homework_13/robot_ws && \
	pwd && \
	colcon build --symlink-install --packages-select antidrone_turret

AntiDroneTurretApp/format-code:
	find ./homework_13 -type f -name "*.cpp" -exec clang-format -i {} +
	find ./homework_13 -type f -name "*.hpp" -exec clang-format -i {} +

AntiDroneTurretApp/format-cmake:
	find ./homework_13 -type f -name "CMakeLists.txt" -exec cmake-format -i {} +

AntiDroneTurretApp/format: format-code format-cmake

AntiDroneTurretApp/format-ci:
	find ./homework_13 -type f -name "*.cpp" \
		-not -path "./homework_13/robot_ws/src/antidrone_turret/src/target_track_publisher_node.cpp" \
		-not -path "./homework_13/robot_ws/src/antidrone_turret/src/actuator_node.cpp" \
		-exec clang-format --dry-run -Werror {} +
	find ./homework_13 -type f -name "*.hpp" \
		-not -path "./homework_13/robot_ws/src/antidrone_turret/include/antidrone_turret/target_sequence.hpp" \
		-not -path "./homework_13/robot_ws/src/antidrone_turret/include/antidrone_turret/target_track_loader.hpp" \
		-not -path "./homework_13/robot_ws/src/antidrone_turret/include/antidrone_turret/target_track_simulation.hpp" \
		-exec clang-format --dry-run -Werror {} +

AntiDroneTurretApp/rqt:
	ros2 run rqt_gui rqt_gui

AntiDroneTurretApp/listen-turret-status:
	cd ./homework_13/robot_ws && \
	source install/setup.bash && \
	ros2 topic echo /turret/status

AntiDroneTurretApp/test: AntiDroneTurretApp/build
	cd ./homework_13/robot_ws && \
	source install/setup.bash && \
	colcon test --packages-select antidrone_turret && \
	colcon test-result --verbose

AntiDroneTurretApp/tests-ci: AntiDroneTurretApp/build-ci
	cd ./homework_13/robot_ws && \
	source install/setup.bash && \
	colcon test --packages-select antidrone_turret && \
	colcon test-result --verbose
