cmake_minimum_required(VERSION 3.24)
project(HashMap)

set(CMAKE_CXX_STANDARD 17)

add_executable(main.cpp unit-test.cpp PureHashMap.h RobinHoodHashMap_WA52.h)
