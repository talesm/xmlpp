cmake_minimum_required(VERSION 3.1)

project(Xmlpp VERSION 0.1)

add_library(xmlpp INTERFACE
)

target_include_directories(xmlpp
    INTERFACE include/
)

add_subdirectory(test)