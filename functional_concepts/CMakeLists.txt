cmake_minimum_required(VERSION 3.18)

project(playground)

enable_language(CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS ON)

add_executable(${PROJECT_NAME}
    src/main.cpp)
