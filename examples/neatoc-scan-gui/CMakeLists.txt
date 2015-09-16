# Set project name
project("neatoc-scan-gui")

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

find_package(wxWidgets REQUIRED core base)
include(${wxWidgets_USE_FILE})
include_directories(${wxWidgets_INCLUDE_DIRS})

add_definitions(${wxWidgets_CXX_FLAGS})

file(GLOB_RECURSE CPPS "src/*.cpp")
file(GLOB_RECURSE HPPS "include/*.hpp")

include_directories("include")

add_executable(${PROJECT_NAME} ${CPPS} ${HPPS})

target_link_libraries(${PROJECT_NAME} boost_system)
target_link_libraries(${PROJECT_NAME} ${wxWidgets_LIBRARIES})
target_link_libraries(${PROJECT_NAME} neatoc)
