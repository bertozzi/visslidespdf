cmake_minimum_required(VERSION 2.8)
project(lab4_2)

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_definitions(-std=gnu++0x)

add_executable(lab4_2 main.cpp)

target_link_libraries( lab4_2 ${OpenCV_LIBS} )
install(TARGETS lab4_2 RUNTIME DESTINATION bin)
