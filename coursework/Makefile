SHELL := /bin/bash
SCENARIO ?= branching_trench

.PHONY: checker

build:	
	. /opt/ros/jazzy/setup.bash
	cd ./robot_ws && \
	colcon build --symlink-install --packages-select ballistics_simulator
# --cmake-args -DCMAKE_BUILD_TYPE=Debug
launch: build
	cd ./robot_ws && \
	source install/setup.bash && \
	ros2 launch ballistics_simulator system.launch.py scenario:=$(SCENARIO).yaml

checker:
	./checker/checker_pi_armv6 2 --hw --uart /dev/ttyAMA0 --gpiochip gpiochip0 --start-line 27 --drop-line 22

rqt:
	ros2 run rqt_gui rqt_gui

test: build
	cd ./coursework/robot_ws && \
	source install/setup.bash && \
	colcon test --packages-select ballistics_simulator && \
	colcon test-result --verbose

bag_init:
	cd ./coursework && \
	rm -rf bags/
	mkdir -p bags/

bag_record:	
	cd ./coursework/robot_ws && \
	source install/setup.bash && \
	ros2 bag record -a -o ../bags/$(SCENARIO)

bag_info:
	cd ./coursework/robot_ws && \
	source install/setup.bash && \
	ros2 bag info ../bags/$(SCENARIO)

bag_play:
	cd ./coursework/robot_ws && \
	source install/setup.bash && \
	ros2 bag play ../bags/$(SCENARIO)

bag_watch_robot_metrics:
	cd ./coursework/robot_ws && \
	source install/setup.bash && \
	ros2 topic echo /robot/metrics
