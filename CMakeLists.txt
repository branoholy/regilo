cmake_minimum_required(VERSION 2.8)

# For compatibility with PROJECT_VERSION* variables
if((${CMAKE_VERSION} VERSION_GREATER "3.0") OR (${CMAKE_VERSION} VERSION_EQUAL "3.0"))
	cmake_policy(SET CMP0048 OLD)
endif()

# Set project name
project("regilo")

# Set project version
set(PROJECT_VERSION_MAJOR "2")
set(PROJECT_VERSION_MINOR "3")
set(PROJECT_VERSION_PATCH "2")
set(PROJECT_VERSION_TWEAK "")
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")

if(${PROJECT_VERSION_PATCH})
	set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
endif()

if(${PROJECT_VERSION_TWEAK})
	set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}")
endif()

# Set options
option(build-library "Build the library even if only the headers are installed")
option(install-library "Install the library only (no headers)")
option(install-headers "Install the headers only (no library)")
option(example "Build the console example")
option(example-gui "Build the GUI example")
option(examples "Build all examples")
option(examples-only "Build only examples (not the library)")

option(INSTALL_LIB_DIR "Installation directory for libraries")
if(${INSTALL_LIB_DIR} STREQUAL "OFF")
	set(INSTALL_LIB_DIR "lib")
endif()

# Set build type
if("${CMAKE_BUILD_TYPE}" STREQUAL "")
	set(CMAKE_BUILD_TYPE "Release")
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Set definitions
if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
	add_definitions("-O3")
endif()

add_definitions("-std=c++11")
add_definitions("-Wall -Wextra -pedantic")

# Find libraries
find_package(Threads)

find_package(Boost 1.54 COMPONENTS system REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

if(${examples-only} STREQUAL "OFF")
	# Include headers
	include_directories("include")

	# Build library
	if((${build-library}) OR (${install-headers} STREQUAL "OFF"))
		# Find source code
		file(GLOB_RECURSE CPPS "src/regilo/*.cpp")
		file(GLOB_RECURSE HPPS "include/regilo/*.hpp")

		# Configure files
		configure_file("src/regilo/version.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/gen/regilo/version.cpp")
		list(APPEND CPPS "${CMAKE_CURRENT_BINARY_DIR}/gen/regilo/version.cpp")

		# Create library
		add_library(${PROJECT_NAME} SHARED ${CPPS} ${HPPS})
		set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")

		# Link libraries
		target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
		target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
	endif()

	# Install library
	if(${install-headers} STREQUAL "OFF")
		install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${INSTALL_LIB_DIR})
	endif()

	# Install headers
	if(${install-library} STREQUAL "OFF")
		install(DIRECTORY include/${PROJECT_NAME}/ DESTINATION include/${PROJECT_NAME})
	endif()
endif()

# Add examples
if((${example}) OR (${examples}))
	add_subdirectory("examples/regilo-scan")
endif()

if((${example-gui}) OR (${examples}))
	add_subdirectory("examples/regilo-visual")
endif()

# Add uninstall
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake" IMMEDIATE @ONLY)
add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake)
