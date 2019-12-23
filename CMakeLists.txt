cmake_minimum_required(VERSION 3.5)
project(xsocket)

message(STATUS "build for OS: ${CMAKE_SYSTEM}")
message(STATUS "root source directory is ${PROJECT_SOURCE_DIR}")
message(STATUS "current source directory is ${CMAKE_CURRENT_SOURCE_DIR}")

set(CMAKE_CXX_FLAGS "-std=c++11")

set(SRCS ../src/sample.cpp
		../src/xsocket.h
		../src/xsocket.cpp
	)

add_executable(xsocket ${SRCS})
