SHELL := /bin/bash
SCENARIO ?= branching_trench

CourseworkApp/build:
	. /opt/ros/jazzy/setup.bash
	cd ./coursework/robot_ws && \
	colcon build --symlink-install --packages-select ballistics_simulator

CourseworkApp/launch: CourseworkApp/build
	cd ./coursework/robot_ws && \
	source install/setup.bash && \
	ros2 launch ballistics_simulator system.launch.py scenario:=$(SCENARIO).yaml
	# ros2 launch ballistics_simulator system.launch.py scenario:=training_corridor.yaml
	# ros2 launch ballistics_simulator system.launch.py scenario:=dead_end_bunker.yaml
	# ros2 launch ballistics_simulator system.launch.py scenario:=small_rooms.yaml

CourseworkApp/build-ci:
	cd ./coursework/robot_ws && \
	colcon build --symlink-install --packages-select ballistics_simulator

CourseworkApp/rqt:
	ros2 run rqt_gui rqt_gui

CourseworkApp/test: CourseworkApp/build
	cd ./coursework/robot_ws && \
	source install/setup.bash && \
	colcon test --packages-select ballistics_simulator && \
	colcon test-result --verbose

CourseworkApp/tests-ci: CourseworkApp/build-ci
	cd ./coursework/robot_ws && \
	source install/setup.bash && \
	colcon test --packages-select ballistics_simulator && \
	colcon test-result --verbose

CourseworkApp/bag_init:
	cd ./coursework && \
	rm -rf bags/
	mkdir -p bags/

CourseworkApp/bag_record:	
	cd ./coursework/robot_ws && \
	source install/setup.bash && \
	ros2 bag record -a -o ../bags/$(SCENARIO)

CourseworkApp/bag_info:
	cd ./coursework/robot_ws && \
	source install/setup.bash && \
	ros2 bag info ../bags/$(SCENARIO)

CourseworkApp/bag_play:
	cd ./coursework/robot_ws && \
	source install/setup.bash && \
	ros2 bag play ../bags/$(SCENARIO)

CourseworkApp/bag_watch_robot_metrics:
	cd ./coursework/robot_ws && \
	source install/setup.bash && \
	ros2 topic echo /robot/metrics
