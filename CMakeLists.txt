cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(darabonba_core VERSION "1.0.0")

# set c++ version
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# compiler warning
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /w44265 /w44061 /w44062")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

# test
option(ENABLE_UNIT_TESTS "Enable unit tests" ON)

# dependency openssl
find_package(OpenSSL REQUIRED)
# curl
find_package(CURL REQUIRED)
add_subdirectory(external)

include_directories(include)

add_subdirectory(src)

if(ENABLE_UNIT_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Install set up >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> #
message(STATUS "${PROJECT_NAME} : Project will be installed to ${CMAKE_INSTALL_PREFIX}")

include(GNUInstallDirs)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

set(INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR} CACHE PATH "Installation directory for libraries")
set(INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR} CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR} CACHE PATH "Installation directory for header files")
if(WIN32 AND NOT CYGWIN)
  set(DEF_INSTALL_CMAKEDIR CMake)
else()
  set(DEF_INSTALL_CMAKEDIR share/cmake/${PROJECT_NAME})
endif()
set(INSTALL_CMAKEDIR ${DEF_INSTALL_CMAKEDIR} CACHE PATH "Installation directory for CMake files")

foreach(p LIB BIN INCLUDE CMAKE)
  file(TO_NATIVE_PATH ${CMAKE_INSTALL_PREFIX}/${INSTALL_${p}DIR} _path )
  message(STATUS "Installing ${p} components to ${_path}")
  unset(_path)
endforeach()

# Prepare RPATH
file(RELATIVE_PATH _rel ${CMAKE_INSTALL_PREFIX}/${INSTALL_BINDIR} ${CMAKE_INSTALL_PREFIX})
if(APPLE)
  set(_rpath "@loader_path/${_rel}")
else()
  set(_rpath "\$ORIGIN/${_rel}")
endif()
file(TO_NATIVE_PATH "${_rpath}/${INSTALL_LIBDIR}" install_RPATH)

set_target_properties(${PROJECT_NAME}
        PROPERTIES
        MACOSX_RPATH ON
        SKIP_BUILD_RPATH OFF
        BUILD_WITH_INSTALL_RPATH OFF
        INSTALL_RPATH "${install_RPATH}"
        INSTALL_RPATH_USE_LINK_PATH ON)


install(
        TARGETS
        ${PROJECT_NAME}
        EXPORT
        darabonba_coreTargets
        ARCHIVE
        DESTINATION ${INSTALL_LIBDIR}
        COMPONENT lib
        RUNTIME
        DESTINATION ${INSTALL_BINDIR}
        COMPONENT bin
        LIBRARY
        DESTINATION ${INSTALL_LIBDIR}
        COMPONENT lib
        PUBLIC_HEADER
        DESTINATION ${INSTALL_INCLUDEDIR}/darabonba
        COMPONENT dev
)

# install cmake config
install(
        EXPORT
        darabonba_coreTargets
        NAMESPACE
        "darabonba_core::"
        DESTINATION
        ${INSTALL_CMAKEDIR}
        COMPONENT
        dev
)
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)
configure_package_config_file(
        ${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        INSTALL_DESTINATION ${INSTALL_CMAKEDIR}
)
install(
        FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        DESTINATION
        ${INSTALL_CMAKEDIR}
)