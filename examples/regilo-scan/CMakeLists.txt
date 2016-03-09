# Set project name
project("regilo-scan")

# Create executable
add_executable(${PROJECT_NAME} "src/main.cpp")

# Install executable
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

# Link libraries
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
target_link_libraries(${PROJECT_NAME} regilo)
