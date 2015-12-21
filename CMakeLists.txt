cmake_minimum_required(VERSION 2.8)

# Set project name
project("neatoc")

# Set options
option(example "Build the console example")
option(example-gui "Build the GUI example")
option(examples "Build all examples")

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
find_package(Boost 1.54 COMPONENTS system REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

# Find source code
file(GLOB_RECURSE CPPS "src/neatoc/*.cpp")
file(GLOB_RECURSE HPPS "include/neatoc/*.hpp")

# Include headers
include_directories("include")

# Create library
add_library(${PROJECT_NAME} SHARED ${CPPS} ${HPPS})

# Install library
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)
install(DIRECTORY include/${PROJECT_NAME}/ DESTINATION include/${PROJECT_NAME})

# Add uninstall
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake" IMMEDIATE @ONLY)
add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake)

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

# Add examples
if((${example}) OR (${examples}))
    add_subdirectory("examples/neatoc-scan")
endif()

if((${example-gui}) OR (${examples}))
    add_subdirectory("examples/neatoc-scan-gui")
endif()

