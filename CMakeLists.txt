cmake_minimum_required(VERSION 2.8)

# Set project name
project("regilo")

# Set options
option(example "Build the console example")
option(example-gui "Build the GUI example")
option(examples "Build all examples")
option(INSTALL_LIB_DIR "Installation directory for libraries")

# Set build type and definitions
if($ENV{DEBUG})
    # Debug
    set(CMAKE_BUILD_TYPE "Debug")
else()
    # Release
    set(CMAKE_BUILD_TYPE "Release")
    add_definitions("-O3")
endif()

add_definitions("-std=c++11")
add_definitions("-Wall -Wextra -pedantic")

# Find libraries
find_package(Threads)

find_package(Boost 1.54 COMPONENTS system REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

# Find source code
file(GLOB_RECURSE CPPS "src/regilo/*.cpp")
file(GLOB_RECURSE HPPS "include/regilo/*.hpp")

# Include headers
include_directories("include")

# Create library
add_library(${PROJECT_NAME} SHARED ${CPPS} ${HPPS})

# Install library
if(${INSTALL_LIB_DIR} STREQUAL "OFF")
	set(INSTALL_LIB_DIR "lib")
endif()

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${INSTALL_LIB_DIR})
install(DIRECTORY include/${PROJECT_NAME}/ DESTINATION include/${PROJECT_NAME})

# Add uninstall
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake" IMMEDIATE @ONLY)
add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake)

# Link libraries
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

# Add examples
if((${example}) OR (${examples}))
    add_subdirectory("examples/regilo-scan")
endif()

if((${example-gui}) OR (${examples}))
    add_subdirectory("examples/regilo-visual")
endif()

