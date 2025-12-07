file(GLOB_RECURSE CPP_FILES "src/*.cpp")

add_executable(${PROJECT_NAME}Test ${CPP_FILES})

# include
target_include_directories(${PROJECT_NAME}Test
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# googletest
target_link_libraries(${PROJECT_NAME}Test ${PROJECT_NAME} gtest gmock gtest_main)

# On Windows, copy the DLL to the test executable directory so it can be found at runtime
if(WIN32)
  add_custom_command(TARGET ${PROJECT_NAME}Test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:${PROJECT_NAME}>
    $<TARGET_FILE_DIR:${PROJECT_NAME}Test>
    COMMENT "Copying ${PROJECT_NAME} DLL to test executable directory"
  )
endif()

add_test(NAME ${PROJECT_NAME}Test COMMAND ${PROJECT_NAME}Test)
