include("${PROJECT_CONFIG_PATH}/tests/unit/config.cmake")
include("${PROJECT_CONFIG_PATH}/project/config.cmake")

add_executable(${PROJECT_UNIT_TESTS_TARGET_NAME} "${TESTS_UNIT_SRC_LIST}" "${TESTS_UNIT_HEADERS_LIST}")
target_include_directories(${PROJECT_UNIT_TESTS_TARGET_NAME} PRIVATE "${TESTS_UNIT_INCLUDE_DIR}")
target_include_directories(${PROJECT_UNIT_TESTS_TARGET_NAME} PRIVATE "${PROJECT_INCLUDE_DIR}")

target_link_libraries(${PROJECT_UNIT_TESTS_TARGET_NAME} PRIVATE CONAN_PKG::gtest CONAN_PKG::spdlog)

# Group files into proper folders - for IDE
GROUP_FILES("${TESTS_UNIT_HEADERS_LIST}" "${TESTS_UNIT_INCLUDE_DIR}")
GROUP_FILES("${TESTS_UNIT_SRC_LIST}" "${TESTS_UNIT_SRC_DIR}")

target_precompile_headers(
	${PROJECT_UNIT_TESTS_TARGET_NAME}
	PRIVATE
		<string>
		<vector>
		<memory>
		<cstdint>
		<array>
)
