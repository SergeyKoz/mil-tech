I2CApp/build:
	cmake --preset debug && cmake --build --preset debug --target i2c_app --target i2c_app

I2CApp/format-code:
	find ./homework_16 -type f -name "*.cpp" -exec clang-format -i {} +
	find ./homework_16 -type f -name "*.hpp" -exec clang-format -i {} +

I2CApp/format-cmake:
	find ./homework_16 -type f -name "CMakeLists.txt" -exec cmake-format -i {} +

I2CApp/format: format-code format-cmake

I2CApp/quality: build
	clang-tidy -p ./build/debug --config-file=.devcontainer/.clang-tidy $$(find ./homework_16 -name '*.cpp')

I2CApp/quality-ci:
	clang-tidy -p ./build/debug --config-file=.devcontainer/.clang-tidy -fix $$(find ./homework_16 -name '*.cpp')

I2CApp/format-ci:
	find ./homework_16 -type f -name "*.cpp" -exec clang-format --dry-run -Werror {} +
	find ./homework_16 -type f -name "*.hpp" -exec clang-format --dry-run -Werror {} +
