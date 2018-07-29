cmake_minimum_required(VERSION 3.5)
project(lock)

option(RUN_TESTS "Run unit tests" OFF)
if(RUN_TESTS)
  add_subdirectory(test)
endif()

