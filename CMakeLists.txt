cmake_minimum_required(VERSION 3.13)

project(darabonba_core VERSION "1.0.0" LANGUAGES CXX)

# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Options >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> #
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(ENABLE_UNIT_TESTS "Enable unit tests" OFF)
option(ENABLE_RTTI "Enable Run-Time Type Information" ON)
option(MINIMIZE_SIZE "Minimize binary size (may affect performance)" OFF)

# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< C++ Standard >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> #
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Platform Setup >>>>>>>>>>>>>>>>>>>>>>>>>>>> #
if(MSVC)
  # Standardize MSVC runtime (MD/MDd for shared, MT/MTd for static)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS -DNOMINMAX)
  add_compile_options(/W4 /WX /Zc:__cplusplus /utf-8)
  
  # Runtime library selection
  if(BUILD_SHARED_LIBS)
    # Use dynamic runtime for shared libraries
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
  else()
    # Use static runtime for static libraries
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  endif()
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Werror)
  # Enable additional deprecation warnings for better C++ compliance
  add_compile_options(-Wdeprecated -Wdeprecated-copy)
  
  # RTTI control for GCC/Clang
  if(NOT ENABLE_RTTI)
    add_compile_options(-fno-rtti)
  endif()
  
  # Size optimization
  if(MINIMIZE_SIZE)
    add_compile_options(-Os -ffunction-sections -fdata-sections)
    if(NOT APPLE)
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
      set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections")
    endif()
  endif()
endif()

# For static builds, prefer static OpenSSL but use dynamic curl
# Note: Static libcurl on Ubuntu has too many transitive dependencies (nghttp2, ssh, psl, 
# gssapi, rtmp, idn2, unistring, etc.) that make full static linking impractical.
# If you need fully static builds, compile a minimal libcurl yourself or use vcpkg.
if(NOT BUILD_SHARED_LIBS)
  set(OPENSSL_USE_STATIC_LIBS ON)
  # Don't set CURL_STATICLIB or CMAKE_FIND_LIBRARY_SUFFIXES - use dynamic libcurl
endif()

# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Dependencies >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> #
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Target Setup >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> #
# The library target is defined in src/CMakeLists.txt
add_subdirectory(src)

# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Subdirectories >>>>>>>>>>>>>>>>>>>>>>>>>>>> #
if(ENABLE_UNIT_TESTS)
  enable_testing()
  add_subdirectory(external)
  add_subdirectory(tests)
endif()

# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Installation >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> #
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(INSTALL_CMAKEDIR ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT ${PROJECT_NAME}Targets
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${INSTALL_CMAKEDIR}
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Configure the Config file from template using configure_package_config_file
# This properly handles @PACKAGE_INIT@ and provides check_required_components
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION ${INSTALL_CMAKEDIR}
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION ${INSTALL_CMAKEDIR}
)
