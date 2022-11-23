cmake_minimum_required(VERSION 2.8)
project(A1)

find_package(OpenCV REQUIRED)

add_definitions("-std=c++11 -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g")

add_executable(a1 a1.cpp)
target_link_libraries(a1 ${OpenCV_LIBS})
install(TARGETS a1 RUNTIME DESTINATION bin)


