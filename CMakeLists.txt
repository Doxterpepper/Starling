cmake_minimum_required(VERSION 3.10)
project(starling)

add_subdirectory(src)
add_subdirectory(test)
set (CMAKE_CXX_STANDARD 20)
#set(CMAKE_BUILD_TYPE Debug)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

message(CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}")