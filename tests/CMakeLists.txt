# Set project name
project("tests")

# Find source code
file(GLOB_RECURSE CPPS "src/*.cpp")

# Create executable
add_executable(${PROJECT_NAME} ${CPPS})

# Link libraries
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
target_link_libraries(${PROJECT_NAME} regilo)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND
	${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/data $<TARGET_FILE_DIR:${PROJECT_NAME}>/data)
