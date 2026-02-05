# 1. Source files - Explicitly listing files is preferred, but GLOB is kept if you have many files
file(GLOB_RECURSE CPP_FILES "*.cpp")

# 2. Add library - The target name is inherited from PROJECT_NAME in the root
add_library(${PROJECT_NAME} ${CPP_FILES})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# 3. Public header directories
# Using CMAKE_CURRENT_SOURCE_DIR to ensure paths are absolute and robust
target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# 4. Link dependencies 
# We assume the root CMakeLists.txt has already found these packages.
# Using Targets (::) ensures that include directories and compile definitions are inherited.
# Note: For static linking, order matters! Libraries that depend on others must come first.
# CURL depends on OpenSSL, so CURL must be listed before OpenSSL.
target_link_libraries(${PROJECT_NAME}
  PUBLIC
    CURL::libcurl
    OpenSSL::SSL
    OpenSSL::Crypto
)

# 5. Conditional dependencies for static builds
if(NOT BUILD_SHARED_LIBS)
  # ZLIB (commonly needed for static curl)
  find_package(ZLIB QUIET)
  if(TARGET ZLIB::ZLIB)
    target_link_libraries(${PROJECT_NAME} PUBLIC ZLIB::ZLIB)
  endif()
  
  # On Linux, static linking may need additional system libraries
  if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PUBLIC dl pthread)
  endif()
endif()

# UUID (Linux specific)
if(UNIX AND NOT APPLE)
  # Try modern PkgConfig first for uuid
  find_package(PkgConfig QUIET)
  pkg_check_modules(UUID uuid)
  if(UUID_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${UUID_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${UUID_INCLUDE_DIRS})
  else()
    # Fallback to manual search
    find_path(UUID_INCLUDE_DIR uuid/uuid.h)
    find_library(UUID_LIBRARY uuid)
    if(UUID_INCLUDE_DIR AND UUID_LIBRARY)
      target_include_directories(${PROJECT_NAME} PRIVATE ${UUID_INCLUDE_DIR})
      target_link_libraries(${PROJECT_NAME} PRIVATE ${UUID_LIBRARY})
    endif()
  endif()
endif()

# macOS specific Frameworks
if(APPLE)
  target_link_libraries(${PROJECT_NAME} PUBLIC "-framework CoreFoundation" "-framework Security")
endif()

# Windows specific libraries
if(WIN32)
  target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()

# 6. Target Properties and Compiler Definitions
set_target_properties(${PROJECT_NAME} PROPERTIES
  POSITION_INDEPENDENT_CODE ON
  WINDOWS_EXPORT_ALL_SYMBOLS ON
  SOVERSION ${PROJECT_VERSION_MAJOR}
  VERSION ${PROJECT_VERSION}
)

if(WIN32)
  target_compile_definitions(${PROJECT_NAME} PRIVATE DARABONBA_CORE_BUILD)
  # For CMake 3.13, we ensure the MD/MDd runtime is set
  target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Release>:/MD>
    $<$<CONFIG:Debug>:/MDd>
  )
endif()
