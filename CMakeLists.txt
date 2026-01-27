cmake_minimum_required(VERSION 3.13)

project(darabonba_core VERSION "1.0.0" LANGUAGES CXX)

# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Options >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> #
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(ENABLE_UNIT_TESTS "Enable unit tests" OFF)

# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< C++ Standard >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> #
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Platform Setup >>>>>>>>>>>>>>>>>>>>>>>>>>>> #
if(MSVC)
  # Standardize MSVC runtime (MD/MDd)
  # For CMake 3.15+, use CMAKE_MSVC_RUNTIME_LIBRARY. 
  # For 3.13, we use a more robust target-based approach later.
  add_definitions(-D_CRT_SECURE_NO_WARNINGS -DNOMINMAX)
  add_compile_options(/W4 /Zc:__cplusplus /utf-8)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# For static builds, prefer static dependencies
if(NOT BUILD_SHARED_LIBS)
  set(OPENSSL_USE_STATIC_LIBS ON)
  set(CURL_STATICLIB ON)
endif()

# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Dependencies >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> #
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Target Setup >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> #
# Assuming the library target is defined here or in src
# For clarity, we define the target here if source files are known, 
# or ensure src/CMakeLists.txt uses the following modern commands.

file(GLOB_RECURSE SRC_FILES "src/*.cpp")
add_library(${PROJECT_NAME} ${SRC_FILES})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# --- THE MOST IMPORTANT FIX: Usage Requirements ---
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        OpenSSL::SSL
        OpenSSL::Crypto
        CURL::libcurl
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()

# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Properties >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> #
set_target_properties(${PROJECT_NAME} PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    SOVERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION}
)

# Handle MSVC runtime for CMake 3.13
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:/MD>
        $<$<CONFIG:Debug>:/MDd>
    )
endif()

# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Subdirectories >>>>>>>>>>>>>>>>>>>>>>>>>>>> #
if(ENABLE_UNIT_TESTS)
  enable_testing()
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

# Generate the Config file that downstream projects will use
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
"include(CMakeFindDependencyMacro)\n"
"find_dependency(OpenSSL)\n"
"find_dependency(CURL)\n"
"include(\"\${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}Targets.cmake\")\n"
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION ${INSTALL_CMAKEDIR}
)
