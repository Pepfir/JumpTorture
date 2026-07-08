cmake_minimum_required(VERSION 3.27.9)
project(jump_torture)

find_package(Geode REQUIRED)

add_library(${PROJECT_NAME} SHARED
    src/main.cpp
    src/JumpLayer.cpp
)

setup_geode_mod(${PROJECT_NAME})