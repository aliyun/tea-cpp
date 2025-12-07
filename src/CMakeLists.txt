file(GLOB_RECURSE CPP_FILES "*.cpp")

add_library(${PROJECT_NAME} SHARED ${CPP_FILES})

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
