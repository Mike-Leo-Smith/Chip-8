cmake_minimum_required(VERSION 2.8)
project(Chip_8)

find_package(OpenCV REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp chip8.cpp chip8.h)
add_executable(Chip_8 ${SOURCE_FILES} chip8.cpp chip8.h)

target_link_libraries(Chip_8 ${OpenCV_LIBS})