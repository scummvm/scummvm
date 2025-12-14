cmake_minimum_required(VERSION 3.23)

project(ScummVM)

add_library(base)

target_sources(base PRIVATE test_new_standards.cpp main.cpp commandLine.cpp
                            plugins.cpp version.cpp)

target_link_libraries(base PRIVATE engines gui backends audio graphics)
