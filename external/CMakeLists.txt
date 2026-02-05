if(ENABLE_UNIT_TESTS)
  # Force GoogleTest to use static libraries on Windows to avoid DLL exit issues
  if(WIN32)
    set(BUILD_SHARED_LIBS_SAVED ${BUILD_SHARED_LIBS})
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
    
    # Force GoogleTest to use dynamic runtime library (/MD) to match our project
    set(gtest_force_shared_crt ON CACHE BOOL "Use shared (DLL) run-time lib even when Google Test is built as static lib." FORCE)
  endif()
  
  # Suppress deprecation warnings from GoogleTest v1.12.1 (last C++11 compatible version)
  set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)
  add_subdirectory(googletest)
  set(CMAKE_WARN_DEPRECATED ON CACHE BOOL "" FORCE)
  
  # Restore original BUILD_SHARED_LIBS setting
  if(WIN32)
    set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_SAVED} CACHE BOOL "Build shared libraries" FORCE)
  endif()
endif()
