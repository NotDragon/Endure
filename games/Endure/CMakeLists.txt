cmake_minimum_required(VERSION 3.23)
project(Endure)

set(CMAKE_CXX_STANDARD 20)

add_library(Endure main.cpp lib/libraylib.a include/json.hpp include/raylib.h include/raymath.h include/resources.h include/rlgl.h include/structs.h include/declarations.h include/function_definitions.h include/save_system.h include/Sprite.h)

include_directories(./include)