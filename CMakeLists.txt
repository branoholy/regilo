cmake_minimum_required(VERSION 2.8)

# Set project name
project("neatoc")

# Set build type and definitions
if($ENV{DEBUG})
    # Debug
    set(CMAKE_BUILD_TYPE "Debug")
else($ENV{DEBUG})
    # Release
    set(CMAKE_BUILD_TYPE "Release")
    add_definitions("-O3")
endif($ENV{DEBUG})

add_definitions("-std=c++11")
add_definitions("-Wall -Wextra -pedantic")

file(GLOB_RECURSE CPPS "src/neatoc/*.cpp")
file(GLOB_RECURSE HPPS "include/neatoc/*.hpp")

include_directories("include")

add_library(${PROJECT_NAME} SHARED ${CPPS} ${HPPS})

add_subdirectory("examples")
