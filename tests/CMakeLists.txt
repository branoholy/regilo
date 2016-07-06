# Set project name
project("tests")

# Create executable
add_executable(${PROJECT_NAME} "src/main.cpp")

# Link libraries
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
target_link_libraries(${PROJECT_NAME} regilo)
