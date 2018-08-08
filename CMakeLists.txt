cmake_minimum_required(VERSION 3.5)
project(lock)

option(LOCK_TESTS "Run unit tests" OFF)
if(LOCK_TESTS)
  add_subdirectory(test)
endif()

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include)
