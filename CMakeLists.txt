cmake_minimum_required(VERSION 3.16)
project(imu-lab LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
add_executable(imu main.cpp attitude.cpp)