file(GLOB_RECURSE CPP_FILES "*.cpp")

add_library(${PROJECT_NAME} SHARED ${CPP_FILES})

if(WIN32)
  # Ensure MSVC emits the import library by exporting all public symbols
  set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
  target_compile_definitions(${PROJECT_NAME} PRIVATE DARABONBA_CORE_BUILD)
endif()

# openssl
target_link_libraries(${PROJECT_NAME} OpenSSL::SSL OpenSSL::Crypto)

# libcurl
target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARIES})

# uuid
if(UNIX AND NOT APPLE)
  find_path(UUID_INCLUDE_DIR uuid/uuid.h)
  find_library(UUID_LIBRARY uuid)

  if(UUID_INCLUDE_DIR AND UUID_LIBRARY)
    target_include_directories(${PROJECT_NAME} PRIVATE ${UUID_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${UUID_LIBRARY})
  endif()
endif()
