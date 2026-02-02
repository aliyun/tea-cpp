file(GLOB_RECURSE CPP_FILES "src/*.cpp")

add_executable(${PROJECT_NAME}Test ${CPP_FILES})

# include
target_include_directories(${PROJECT_NAME}Test
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# googletest
target_link_libraries(${PROJECT_NAME}Test ${PROJECT_NAME} gtest gmock gtest_main)

# On Windows, ensure we use dynamic runtime and exclude static runtime libraries
if(MSVC)
  # Force dynamic runtime library (/MD for Release, /MDd for Debug)
  target_compile_options(${PROJECT_NAME}Test PRIVATE
    $<$<CONFIG:Release>:/MD>
    $<$<CONFIG:Debug>:/MDd>
  )
  
  # Explicitly exclude static C++ runtime libraries to avoid conflicts
  target_link_options(${PROJECT_NAME}Test PRIVATE
    /NODEFAULTLIB:libcmt
    /NODEFAULTLIB:libcmtd
    /NODEFAULTLIB:libcpmt
    /NODEFAULTLIB:libcpmtd
  )
endif()

# On Windows, copy DLLs to the test executable directory so they can be found at runtime
if(WIN32 AND BUILD_SHARED_LIBS)
  # Copy darabonba_core DLL
  add_custom_command(TARGET ${PROJECT_NAME}Test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:${PROJECT_NAME}>
    $<TARGET_FILE_DIR:${PROJECT_NAME}Test>
    COMMENT "Copying ${PROJECT_NAME} DLL to test executable directory"
  )
  
  # Copy dependency DLLs from vcpkg
  # Use CMake's built-in capability to copy runtime dependencies (CMake 3.21+)
  # For older CMake, we manually copy known DLLs
  if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.21")
    add_custom_command(TARGET ${PROJECT_NAME}Test POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}Test> $<TARGET_FILE_DIR:${PROJECT_NAME}Test>
      COMMAND_EXPAND_LISTS
      COMMENT "Copying runtime DLLs to test directory"
    )
  else()
    # For CMake < 3.21, manually copy DLLs from dependency targets
    # Get the DLL files from OpenSSL and CURL targets
    if(TARGET OpenSSL::SSL)
      add_custom_command(TARGET ${PROJECT_NAME}Test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:OpenSSL::SSL>
        $<TARGET_FILE_DIR:${PROJECT_NAME}Test>
        COMMENT "Copying OpenSSL SSL DLL"
      )
    endif()
    if(TARGET OpenSSL::Crypto)
      add_custom_command(TARGET ${PROJECT_NAME}Test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:OpenSSL::Crypto>
        $<TARGET_FILE_DIR:${PROJECT_NAME}Test>
        COMMENT "Copying OpenSSL Crypto DLL"
      )
    endif()
    if(TARGET CURL::libcurl)
      add_custom_command(TARGET ${PROJECT_NAME}Test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:CURL::libcurl>
        $<TARGET_FILE_DIR:${PROJECT_NAME}Test>
        COMMENT "Copying CURL DLL"
      )
    endif()
  endif()
endif()

add_test(NAME ${PROJECT_NAME}Test COMMAND ${PROJECT_NAME}Test)
