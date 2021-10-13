set(PROJECT_EXE_TARGET_NAME VulkanRenderer)

set(PROJECT_INCLUDE_DIR "${${PROJECT_MAIN_NAME}_SOURCE_DIR}/include")
set(PROJECT_SRC_DIR "${${PROJECT_MAIN_NAME}_SOURCE_DIR}/src")

set(PROJECT_HEADERS_PREFIX "${PROJECT_INCLUDE_DIR}/${PROJECT_EXE_TARGET_NAME}/")
set(PROJECT_SRC_PREFIX "${PROJECT_SRC_DIR}/${PROJECT_EXE_TARGET_NAME}/")

set(
	PROJECT_HEADERS_LIST
	"Application.h"
    "Paths.h"
    "Vulkan/Initializer.h"
    "Vulkan/Shader.h"
    "Vendors/tiny_obj_loader.h"
    "Vulkan/Vulkan.h"
)
set(
	PROJECT_SRC_LIST
    "Application.cpp"
	"main.cpp"
    "Vulkan/Initializer.cpp"
    "Vulkan/Shader.cpp"
    "Vulkan/Vulkan.cpp"
)

list(TRANSFORM PROJECT_SRC_LIST PREPEND ${PROJECT_SRC_PREFIX})
list(TRANSFORM PROJECT_HEADERS_LIST PREPEND ${PROJECT_HEADERS_PREFIX})

add_executable(${PROJECT_EXE_TARGET_NAME} "${PROJECT_SRC_LIST}" "${PROJECT_HEADERS_LIST}")
target_include_directories(${PROJECT_EXE_TARGET_NAME} PUBLIC "${PROJECT_INCLUDE_DIR}")
target_compile_features(${PROJECT_EXE_TARGET_NAME} PUBLIC cxx_std_17)

# Add linking to libraries from Conan
target_link_libraries(
	${PROJECT_EXE_TARGET_NAME}
	PRIVATE
		CONAN_PKG::spdlog
		CONAN_PKG::glm
		CONAN_PKG::glfw
		Vulkan::Vulkan
		CONAN_PKG::stb
)

# Group files into proper folders - for IDE
group_files("${PROJECT_HEADERS_LIST}" "${PROJECT_INCLUDE_DIR}")
group_files("${PROJECT_SRC_LIST}" "${PROJECT_SRC_DIR}")

set_compiler_options(${PROJECT_EXE_TARGET_NAME})

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
