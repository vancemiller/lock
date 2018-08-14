cmake_minimum_required(VERSION 3.5)
project(lock)

option(LOCK_TESTS "Run unit tests" OFF)
if(LOCK_TESTS)
  add_subdirectory(test)
endif()

find_package(Threads REQUIRED)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include)
target_link_libraries(${PROJECT_NAME} INTERFACE pthread)
