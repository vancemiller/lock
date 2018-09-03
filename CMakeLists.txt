cmake_minimum_required(VERSION 3.5)
project(lock)

option(LOCK_TESTS "Run unit tests" OFF)
if(LOCK_TESTS)
  add_subdirectory(test)
endif()

find_package(Threads REQUIRED)

file(GLOB srcs src/*.cpp)
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include)
target_sources(${PROJECT_NAME} INTERFACE ${srcs})
target_link_libraries(${PROJECT_NAME} INTERFACE pthread)
