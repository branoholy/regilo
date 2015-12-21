# Set project name
project("neatoc-scan")

# Create executable
add_executable(${PROJECT_NAME} "src/neatoc-scan.cpp")

# Install executable
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

# Link libraries
target_link_libraries(${PROJECT_NAME} neatoc)

