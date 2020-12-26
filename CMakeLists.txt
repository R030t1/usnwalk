cmake_minimum_required(VERSION 3.0.0)
project(usnwalk VERSION 0.1.0)

set(usnwalk PROPERTY CXX_STANDARD_REQUIRED 20)
set(CMAKE_CXX_FLAGS "/std:c++latest")
add_executable(usnwalk usnwalk.cc)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
