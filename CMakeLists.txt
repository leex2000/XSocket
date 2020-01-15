cmake_minimum_required(VERSION 3.5)
project(xsocket)

message(STATUS "build for OS: ${CMAKE_SYSTEM}")
message(STATUS "root source directory is ${PROJECT_SOURCE_DIR}")
message(STATUS "current source directory is ${CMAKE_CURRENT_SOURCE_DIR}")

set(CMAKE_CXX_FLAGS "-std=c++11")

option (SUPPORT_TLS "Support tls connect" ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PROJECT_BINARY_DIR}/bin)

if (SUPPORT_TLS)
	include_directories(
	    ${CMAKE_CURRENT_SOURCE_DIR}/mbedtls/include
		${CMAKE_CURRENT_SOURCE_DIR}/mbedtls/crypto/include
	)

	link_directories(
    	${PROJECT_BINARY_DIR}/bin
	)

	set(ENABLE_PROGRAMS OFF CACHE INTERNAL "set mbedtls sample off")
	add_subdirectory(mbedtls)
endif (SUPPORT_TLS)

#set(LIBPATH ${PROJECT_BINARY_DIR}/mbedtls/library/${CMAKE_BUILD_TYPE})
#message(${LIBPATH})

set(SRCS ../src/sample.cpp
		../src/xsocket.h
		../src/xsocket.cpp
	)

if (SUPPORT_TLS)
	set(SRCS ${SRCS}
		../src/tls_socket.h
		../src/tls_socket.cpp
	)
endif (SUPPORT_TLS)

add_executable(xsocket ${SRCS})

# get compiler
string(REGEX MATCH "Clang" CMAKE_COMPILER_IS_CLANG "${CMAKE_C_COMPILER_ID}")
string(REGEX MATCH "GNU" CMAKE_COMPILER_IS_GNU "${CMAKE_C_COMPILER_ID}")
string(REGEX MATCH "IAR" CMAKE_COMPILER_IS_IAR "${CMAKE_C_COMPILER_ID}")
string(REGEX MATCH "MSVC" CMAKE_COMPILER_IS_MSVC "${CMAKE_C_COMPILER_ID}")

if(CMAKE_COMPILER_IS_MSVC)
	target_link_libraries(xsocket "mbedtls.lib" "mbedx509.lib" "mbedcrypto.lib")
else()
	target_link_libraries(xsocket "mbedtls.so" "mbedx509.so" "mbedcrypto.so")
endif()
