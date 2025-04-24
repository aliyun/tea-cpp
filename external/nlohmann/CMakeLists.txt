# set the header file path of nlohmann::json
set(NLOHMANN_JSON_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)

# export header file path
add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json
                           INTERFACE ${NLOHMANN_JSON_INCLUDE_DIRS})
