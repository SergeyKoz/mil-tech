.PHONY: build

-include ./homework_ballistics/BallisticsApp.Makefile
-include ./homework_13/AntiDroneTurretApp.Makefile
-include ./homework_14/UndergroundWorldApp.Makefile

build:
	cmake --preset debug && cmake --build --preset debug --target ballistics_cli --target ballistics_tests

format-code:
	find ./homework_06 -type f -name "*.cpp" -exec clang-format -i {} +
	find ./homework_06 -type f -name "*.hpp" -exec clang-format -i {} +

format-cmake:
	find ./homework_06 -type f -name "CMakeLists.txt" -exec cmake-format -i {} +

format: format-code format-cmake

test: build
	./build/debug/homework_06/ballistics_tests

quality: build
	clang-tidy -p ./build/debug --config-file=.devcontainer/.clang-tidy $$(find ./homework_06 -name '*.cpp')

quality-ci:
	clang-tidy -p ./build/debug --config-file=.devcontainer/.clang-tidy -fix $$(find ./homework_06 -name '*.cpp')

format-ci:
	find ./homework_06 -type f -name "*.cpp" -exec clang-format --dry-run -Werror {} +
	find ./homework_06 -type f -name "*.hpp" -exec clang-format --dry-run -Werror {} +

test-ci:
# 	./build/debug/homework_06/ballistics_tests
	ctest --test-dir build/debug --output-on-failure