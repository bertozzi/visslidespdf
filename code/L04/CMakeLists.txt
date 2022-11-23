cmake_minimum_required(VERSION 2.8)
project(lab4)

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable(lab4 main.cpp)

target_link_libraries( lab4 ${OpenCV_LIBS} )
install(TARGETS lab4 RUNTIME DESTINATION bin)
