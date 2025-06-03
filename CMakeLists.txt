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
  add_subdirectory(tests)
endif()
