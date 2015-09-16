# Set project name
project("neatoc-scan")

# Set build type and definitions
#if($ENV{DEBUG})
    # Debug
#    set(CMAKE_BUILD_TYPE "Debug")
#else($ENV{DEBUG})
    # Release
#    set(CMAKE_BUILD_TYPE "Release")
#    add_definitions("-O3")
#endif($ENV{DEBUG})

#add_definitions("-std=c++11")
#add_definitions("-Wall -Wextra -pedantic")

add_executable(${PROJECT_NAME} "src/neatoc-scan.cpp")

target_link_libraries(${PROJECT_NAME} boost_system)
target_link_libraries(${PROJECT_NAME} neatoc)
