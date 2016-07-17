# Set project name
project("tests")

# Find libraries
find_package(Boost 1.54 REQUIRED COMPONENTS system unit_test_framework)
include_directories(${Boost_INCLUDE_DIR})

# Include headers
include_directories("include")

# Find source code
file(GLOB_RECURSE CPPS "src/*.cpp")
file(GLOB_RECURSE HPPS "include/*.hpp")

# Create executable
add_executable(${PROJECT_NAME} ${CPPS} ${HPPS})

# Link libraries
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
target_link_libraries(${PROJECT_NAME} regilo)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND
	${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/data $<TARGET_FILE_DIR:${PROJECT_NAME}>/data)
