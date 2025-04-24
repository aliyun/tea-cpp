# set the source and header file paths of tinyxml2
set(TINYXML2_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/tinyxml2/tinyxml2.cpp)
set(TINYXML2_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)

# add tinyxml2 library
add_library(tinyxml2 STATIC ${TINYXML2_SRC})

# export header file path of tinyxml2 library
target_include_directories(tinyxml2 PUBLIC ${TINYXML2_INCLUDE_DIRS})
