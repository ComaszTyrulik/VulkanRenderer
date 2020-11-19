include("${PROJECT_CONFIG_PATH}/project/config.cmake")

add_executable(${PROJECT_EXE_TARGET_NAME} "${PROJECT_SRC_LIST}" "${PROJECT_HEADERS_LIST}")
target_include_directories(${PROJECT_EXE_TARGET_NAME} PUBLIC "${PROJECT_INCLUDE_DIR}")
target_compile_features(${PROJECT_EXE_TARGET_NAME} PUBLIC cxx_std_17)

# Add linking to libraries from Conan
target_link_libraries(${PROJECT_EXE_TARGET_NAME} CONAN_PKG::spdlog CONAN_PKG::glm CONAN_PKG::glfw Vulkan::Vulkan)

# Group files into proper folders - for IDE
GROUP_FILES("${PROJECT_HEADERS_LIST}" "${PROJECT_INCLUDE_DIR}")
GROUP_FILES("${PROJECT_SRC_LIST}" "${PROJECT_SRC_DIR}")

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${PROJECT_EXE_TARGET_NAME} PRIVATE /EHsc /W4 /c)
endif()

target_precompile_headers(
	${PROJECT_EXE_TARGET_NAME}
	PRIVATE
		<string>
		<vector>
		<memory>
		<cstdint>
		<array>
		<unordered_map>
		<map>
)

configure_file(
	"${PROJECT_INCLUDE_DIR}/VulkanRenderer/Paths.h.in"
	"${PROJECT_INCLUDE_DIR}/VulkanRenderer/Paths.h"
)
