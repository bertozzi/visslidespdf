cmake_minimum_required(VERSION 2.8)
project(simple)

find_package(OpenCV REQUIRED)

add_definitions("-std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

add_executable(simple simple.cpp)
target_link_libraries(simple ${OpenCV_LIBS})
install(TARGETS simple RUNTIME DESTINATION bin)

add_executable(prevframe prevframe.cpp)
target_link_libraries(prevframe ${OpenCV_LIBS})
install(TARGETS prevframe RUNTIME DESTINATION bin)

add_executable(running running.cpp)
target_link_libraries(running ${OpenCV_LIBS})
install(TARGETS running RUNTIME DESTINATION bin)

add_executable(exprunning exprunning.cpp)
target_link_libraries(exprunning ${OpenCV_LIBS})
install(TARGETS exprunning RUNTIME DESTINATION bin)


