BallisticsApp/build:
	cmake --preset debug && cmake --build --preset debug --target ballistics_app --target ballistics_app

BallisticsApp/format-code:
	find ./homework_ballistics -type f -name "*.cpp" -exec clang-format -i {} +
	find ./homework_ballistics -type f -name "*.hpp" -exec clang-format -i {} +

BallisticsApp/format-cmake:
	find ./homework_ballistics -type f -name "CMakeLists.txt" -exec cmake-format -i {} +

BallisticsApp/format: format-code format-cmake

BallisticsApp/quality: build
	clang-tidy -p ./build/debug --config-file=.devcontainer/.clang-tidy $$(find ./homework_ballistics -name '*.cpp' -not -path '*CheckerRpiGPIO.cpp')

BallisticsApp/quality-ci:
	clang-tidy -p ./build/debug --config-file=.devcontainer/.clang-tidy -fix $$(find ./homework_ballistics -name '*.cpp' -not -path '*CheckerRpiGPIO.cpp')

BallisticsApp/format-ci:
	find ./homework_ballistics -type f -name "*.cpp" -exec clang-format --dry-run -Werror {} +
	find ./homework_ballistics -type f -name "*.hpp" -exec clang-format --dry-run -Werror {} +
