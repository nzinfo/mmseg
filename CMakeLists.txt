# Project
project(mmseg)
set(MAJOR_VERSION 5)
set(MINOR_VERSION 1)
set(PACKAGE mmseg)
set(VERSION_STRING ${MAJOR_VERSION}.${MINOR_VERSION})
set(VERSION ${VERSION_STRING})

# Options. Turn on with 'cmake -Dmyvarname=ON'.
option(test "Build all tests." ON) # Makes boolean 'test' available.


if(COMMAND cmake_policy)
   cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH} )

# CMake
if("${CMAKE_BUILD_TYPE}" STREQUAL "")
   set(CMAKE_BUILD_TYPE     RelWithDebInfo)
endif("${CMAKE_BUILD_TYPE}" STREQUAL "")
set(CMAKE_COLOR_MAKEFILE ON)
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)


add_subdirectory (third_party)

# Setup GTest
#if (test)
#    enable_testing()
#    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
#    add_subdirectory(tests)
#endif()


# Debug build
message("-- Build type: ${CMAKE_BUILD_TYPE}")
if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
   add_definitions(-DDEBUG)
endif(${CMAKE_BUILD_TYPE} STREQUAL Debug)

#include the subdirectory containing our libs
#add_subdirectory (src)
#include_directories(src)

#build language binding
#add_subdirectory (binding)



# install (TARGETS CMakeHelloWorld DESTINATION bin)
