file(GLOB_RECURSE CPP_FILES "src/*.cpp")

add_executable(${PROJECT_NAME}Test ${CPP_FILES})

# include
target_include_directories(${PROJECT_NAME}Test
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# googletest
target_link_libraries(${PROJECT_NAME}Test ${PROJECT_NAME} gtest gmock gtest_main)

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
  if(DEFINED VCPKG_TARGET_TRIPLET OR DEFINED ENV{VCPKG_INSTALLATION_ROOT})
    # Determine vcpkg bin directory
    if(DEFINED VCPKG_TARGET_TRIPLET)
      set(VCPKG_BIN_DIR "$ENV{VCPKG_INSTALLATION_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/bin")
    else()
      # Fallback: assume x64-windows for Windows
      set(VCPKG_BIN_DIR "$ENV{VCPKG_INSTALLATION_ROOT}/installed/x64-windows/bin")
    endif()
    
    # Use a CMake script to copy all DLLs at build time
    add_custom_command(TARGET ${PROJECT_NAME}Test POST_BUILD
      COMMAND ${CMAKE_COMMAND} 
        -DVCPKG_BIN_DIR="${VCPKG_BIN_DIR}"
        -DTARGET_DIR="$<TARGET_FILE_DIR:${PROJECT_NAME}Test>"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/copy_runtime_dlls.cmake"
      COMMENT "Copying vcpkg runtime DLLs to test directory"
      VERBATIM
    )
  endif()
endif()

add_test(NAME ${PROJECT_NAME}Test COMMAND ${PROJECT_NAME}Test)
