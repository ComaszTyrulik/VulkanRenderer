# This function groups files so that they appear in the IDE
# in the exact way as they are present in project (All files and directories are mirrored properly)
# @param IN_FILES - a list of files to be put into a source group
# @param IN_DIRECTORY_PREFIX - the filepath under this prefix will be removed from the absolute path of the given IN_FILES.
#	Thanks to that, only the important directories will be shown in IDE, and not the full, absolute paths.
function(GROUP_FILES IN_FILES IN_DIRECTORY_PREFIX)
	foreach(FILE ${IN_FILES})
		#convert source file to absolute
		get_filename_component(ABSOLUTE_PATH "${FILE}" ABSOLUTE)
		
		# Get the directory of the absolute source file
		get_filename_component(PARENT_DIR "${ABSOLUTE_PATH}" DIRECTORY)
		
		# Remove given directory prefix to make the group
		string(REPLACE "${IN_DIRECTORY_PREFIX}" "" GROUP "${PARENT_DIR}")
		
		# Make sure we are using windows slashes
		string(REPLACE "/" "\\" GROUP "${GROUP}")
		
		# Group into "Source Files" and "Header Files"
		if ("${FILE}" MATCHES ".*\\.cpp")
		  set(GROUP "Source Files${GROUP}")
		elseif("${FILE}" MATCHES ".*\\.h")
		  set(GROUP "Header Files${GROUP}")
		endif()
		
		source_group("${GROUP}" FILES "${FILE}")
	
	endforeach()
endfunction(GROUP_FILES)

# This function sets startup project for the Visual Studio solution.
# This function should be called from the `CMakeLists.txt` file containing the `project()` definition
# @param IN_PROJECT - This parameter should contain the name of the project to be set as the VS Startup project
function(SET_STARTUP_PROJECT IN_PROJECT)
	get_property(IS_VS_STARTUP_PROJECT_SET DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT DEFINED)
	
	if ("${IS_VS_STARTUP_PROJECT_SET}")
		get_property(STARTUP_PROJECT DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT)
		message(STATUS "Startup project is already set to: ${STARTUP_PROJECT}")
	else()
		set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT "${IN_PROJECT}")

		message(STATUS "Startup project set to: ${IN_PROJECT}")
	endif()
endfunction()
