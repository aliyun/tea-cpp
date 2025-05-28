file(GLOB_RECURSE CPP_FILES "*.cpp")

add_library(${PROJECT_NAME} SHARED ${CPP_FILES})

# openssl
target_link_libraries(${PROJECT_NAME} OpenSSL::SSL OpenSSL::Crypto)

# libcurl
target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARIES})

# nlohmann::json tinyxml2
target_link_libraries(${PROJECT_NAME} nlohmann_json tinyxml2)
