# Set project name
project("regilo-visual")

# Find libraries
find_package(wxWidgets REQUIRED core base)
include(${wxWidgets_USE_FILE})
include_directories(${wxWidgets_INCLUDE_DIRS})

add_definitions(${wxWidgets_CXX_FLAGS})

# Find source code
file(GLOB_RECURSE CPPS "src/*.cpp")
file(GLOB_RECURSE HPPS "include/*.hpp")

# Include headers
include_directories("include")

# Create executable
add_executable(${PROJECT_NAME} ${CPPS} ${HPPS})

# Install executable
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(DIRECTORY images DESTINATION share/${PROJECT_NAME})

# Link libraries
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${wxWidgets_LIBRARIES})
target_link_libraries(${PROJECT_NAME} regilo)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND
				   ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/images $<TARGET_FILE_DIR:${PROJECT_NAME}>/images)
