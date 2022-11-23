cmake_minimum_required(VERSION 2.8)
project(morpho)

find_package(OpenCV REQUIRED)

add_definitions("-std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

add_executable(morpho morpho.cpp)

target_link_libraries(morpho ${OpenCV_LIBS})

install(TARGETS morpho RUNTIME DESTINATION bin)
