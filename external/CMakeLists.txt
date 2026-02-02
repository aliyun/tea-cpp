if(ENABLE_UNIT_TESTS)
  # Force GoogleTest to use static libraries on Windows to avoid DLL exit issues
  if(WIN32)
    set(BUILD_SHARED_LIBS_SAVED ${BUILD_SHARED_LIBS})
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
  endif()
  
  add_subdirectory(googletest)
  
  # Restore original BUILD_SHARED_LIBS setting
  if(WIN32)
    set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_SAVED} CACHE BOOL "Build shared libraries" FORCE)
  endif()
endif()
