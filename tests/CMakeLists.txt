file(GLOB_RECURSE CPP_FILES "src/*.cpp")

add_executable(${PROJECT_NAME}Test ${CPP_FILES})

# include
target_include_directories(${PROJECT_NAME}Test
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# googletest
target_link_libraries(${PROJECT_NAME}Test ${PROJECT_NAME} gtest gmock gtest_main)
