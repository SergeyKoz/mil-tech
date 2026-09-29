SHELL := /bin/bash
GCS_HOST ?= 127.0.0.1

CourseworkApp/cli:
	cd ./coursework && \
	docker compose exec ros2-dev bash

CourseworkApp/ardu-cli:
	cd ./coursework && \
	docker compose exec ardupilot-sitl bash

CourseworkApp/up:
	cd ./coursework/robot_ws && \
	export GCS_HOST=$(GCS_HOST) && \
	docker compose up -d

	# 192.168.0.103, 192.168.173.150

CourseworkApp/down:
	cd ./coursework/robot_ws && \
	docker compose down

CourseworkApp/launch:
	docker exec ros2_jazzy_rpi5 make launch

CourseworkApp/checker:
	docker exec ros2_jazzy_rpi5 make checker

CourseworkApp/build-ci:
	cd ./coursework/robot_ws && \
	colcon build --symlink-install --packages-select ballistics_simulator

CourseworkApp/tests-ci: CourseworkApp/build-ci
	cd ./coursework/robot_ws && \
	source install/setup.bash && \
	colcon test --packages-select ballistics_simulator && \
	colcon test-result --verbose
