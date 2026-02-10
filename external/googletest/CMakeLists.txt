cmake_minimum_required(VERSION 3.13)
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  # GIT_REPOSITORY https://gitee.com/mirrors/googletest.git
  # The final version that supports C++ 11
  GIT_TAG v1.12.x)
FetchContent_MakeAvailable(googletest)

# Disable warnings for GoogleTest targets to avoid compilation errors
# These warnings come from GoogleTest's own code, not our code
if(MSVC)
  # Disable specific warnings that cause issues in GoogleTest
  # C4061: enumerator in switch of enum is not explicitly handled by a case label
  set(GTEST_TARGETS gtest gtest_main gmock gmock_main)
  foreach(target IN LISTS GTEST_TARGETS)
    if(TARGET ${target})
      # Disable the specific warning and ensure warnings are not treated as errors
      target_compile_options(${target} PRIVATE 
        /wd4061    # Disable C4061 warning
        /WX-       # Explicitly disable treating warnings as errors
      )
    endif()
  endforeach()
else()
  # For GCC/Clang, disable warnings for GoogleTest
  set(GTEST_TARGETS gtest gtest_main gmock gmock_main)
  foreach(target IN LISTS GTEST_TARGETS)
    if(TARGET ${target})
      target_compile_options(${target} PRIVATE -w)
    endif()
  endforeach()
endif()
