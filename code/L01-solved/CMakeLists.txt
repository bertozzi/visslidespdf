cmake_minimum_required(VERSION 2.8)
project(L01)

find_package(OpenCV REQUIRED)

add_definitions("-std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

add_executable(l01 l01.cpp)
target_link_libraries(l01 ${OpenCV_LIBS})
install(TARGETS l01 RUNTIME DESTINATION bin)

