cmake_minimum_required(VERSION 3.14)
include(FetchContent)
FetchContent_Declare(
  googletest
  # GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_REPOSITORY https://gitee.com/mirrors/googletest.git
  # The final version that supports C++ 11
  GIT_TAG v1.12.x)
FetchContent_MakeAvailable(googletest)
