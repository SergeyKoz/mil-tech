SHELL := /bin/bash
SCENARIO ?= branching_trench

UndergroundWorldApp/build:
	. /opt/ros/jazzy/setup.bash
	cd ./homework_14/robot_ws && \
	colcon build --symlink-install --packages-select underground_world

UndergroundWorldApp/launch: UndergroundWorldApp/build
	cd ./homework_14/robot_ws && \
	source install/setup.bash && \
	ros2 launch underground_world system.launch.py scenario:=$(SCENARIO).yaml
	# ros2 launch underground_world system.launch.py scenario:=training_corridor.yaml
	# ros2 launch underground_world system.launch.py scenario:=dead_end_bunker.yaml
	# ros2 launch underground_world system.launch.py scenario:=small_rooms.yaml

UndergroundWorldApp/build-ci:
	cd ./homework_14/robot_ws && \
	colcon build --symlink-install --packages-select underground_world

UndergroundWorldApp/rqt:
	ros2 run rqt_gui rqt_gui

UndergroundWorldApp/test: UndergroundWorldApp/build
	cd ./homework_14/robot_ws && \
	source install/setup.bash && \
	colcon test --packages-select underground_world && \
	colcon test-result --verbose

UndergroundWorldApp/tests-ci: AntiDroneTurretApp/build-ci
	cd ./homework_14/robot_ws && \
	source install/setup.bash && \
	colcon test --packages-select underground_world && \
	colcon test-result --verbose

UndergroundWorldApp/bag_init:
	cd ./homework_14 && \
	rm -rf bags/
	mkdir -p bags/

UndergroundWorldApp/bag_record:	
	cd ./homework_14/robot_ws && \
	source install/setup.bash && \
	ros2 bag record -a -o ../bags/$(SCENARIO)

UndergroundWorldApp/bag_info:
	cd ./homework_14/robot_ws && \
	source install/setup.bash && \
	ros2 bag info ../bags/$(SCENARIO)

UndergroundWorldApp/bag_play:
	cd ./homework_14/robot_ws && \
	source install/setup.bash && \
	ros2 bag play ../bags/$(SCENARIO)

UndergroundWorldApp/bag_watch_robot_metrics:
	cd ./homework_14/robot_ws && \
	source install/setup.bash && \
	ros2 topic echo /robot/metrics
