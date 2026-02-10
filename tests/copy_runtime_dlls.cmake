# CMake script to copy runtime DLLs from vcpkg to test directory
# This script is invoked at build time by tests/CMakeLists.txt

if(NOT DEFINED VCPKG_BIN_DIR OR NOT DEFINED TARGET_DIR)
    message(FATAL_ERROR "VCPKG_BIN_DIR and TARGET_DIR must be defined")
endif()

# Check if vcpkg bin directory exists
if(NOT EXISTS "${VCPKG_BIN_DIR}")
    message(WARNING "vcpkg bin directory not found: ${VCPKG_BIN_DIR}")
    return()
endif()

# Find all DLL files in vcpkg bin directory
file(GLOB DLL_FILES "${VCPKG_BIN_DIR}/*.dll")

if(NOT DLL_FILES)
    message(WARNING "No DLL files found in ${VCPKG_BIN_DIR}")
    return()
endif()

# Copy each DLL to target directory
foreach(DLL_FILE ${DLL_FILES})
    get_filename_component(DLL_NAME "${DLL_FILE}" NAME)
    
    # Skip if already exists and is identical
    set(TARGET_FILE "${TARGET_DIR}/${DLL_NAME}")
    if(EXISTS "${TARGET_FILE}")
        file(MD5 "${DLL_FILE}" SOURCE_MD5)
        file(MD5 "${TARGET_FILE}" TARGET_MD5)
        if("${SOURCE_MD5}" STREQUAL "${TARGET_MD5}")
            continue()
        endif()
    endif()
    
    # Copy the DLL
    message(STATUS "Copying ${DLL_NAME} to test directory")
    file(COPY "${DLL_FILE}" DESTINATION "${TARGET_DIR}")
endforeach()

message(STATUS "Copied ${CMAKE_MATCH_COUNT} DLL files to ${TARGET_DIR}")
