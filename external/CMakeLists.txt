if(ENABLE_UNIT_TESTS)
  # Force GoogleTest to use static libraries on Windows to avoid DLL exit issues
  if(WIN32)
    set(BUILD_SHARED_LIBS_SAVED ${BUILD_SHARED_LIBS})
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
    
    # Force GoogleTest to use dynamic runtime library (/MD) to match our project
    set(gtest_force_shared_crt ON CACHE BOOL "Use shared (DLL) run-time lib even when Google Test is built as static lib." FORCE)
  endif()
  
  add_subdirectory(googletest)
  
  # Restore original BUILD_SHARED_LIBS setting
  if(WIN32)
    set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_SAVED} CACHE BOOL "Build shared libraries" FORCE)
  endif()
endif()
