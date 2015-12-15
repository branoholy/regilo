# Set project name
project("neatoc-scan")

# Find libraries
find_package(Boost 1.54 COMPONENTS system REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

# Create executable
add_executable(${PROJECT_NAME} "src/neatoc-scan.cpp")

# Install executable
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

# Link libraries
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
target_link_libraries(${PROJECT_NAME} neatoc)

