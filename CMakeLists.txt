# This is the cmake-based build system for mmseg 
#
# You may pass various options to cmake:
# -DWANT_(CORE|QTCLIENT|MONO)=(ON|OFF)
#                        : select binaries to build
# -DSTATIC=ON            : Enable static building of mmseg. Use with care.
#
# NOTE: You should remove CMakeCache.txt if you plan to change any of these values!

project(mmseg)

include(CheckFunctionExists)
include(CheckIncludeFile)

# cmake 2.6.2 is required for KDE >=4.2 and should be widespread enough now
cmake_minimum_required(VERSION 2.6.2 FATAL_ERROR)

if(COMMAND cmake_policy)
   cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# Use our own (well, and KDE's) version of some modules
# In particular cmake's own FindQt4 and FindOpenSSL are quite buggy
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
#include(QuasselMacros)

# Various options and variables that can be set on the command line
option(STATIC        "Enable static building (might not be portable)" OFF)

if(APPLE)
  option(DEPLOY        "Mac OS X only! Adds required libs to bundle resources and create a dmg. Note: requires Qt to be built with 10.4u SDK" OFF)
endif(APPLE)

# Some settings imply others
if(STATIC)
  add_definitions(-DSTATIC)
endif(STATIC)

# For static builds, arbitrary extra libs might need to be linked
# Define a comma-separated list here
# e.g. for pgsql, we need -DLINK_EXTRA=pq;crypt
set(LINK_EXTRA "" CACHE STRING "Semicolon-separated list of libraries to be linked")
if(LINK_EXTRA)
  string(REPLACE "," ";" LINK_EXTRA ${LINK_EXTRA})
  link_libraries(${LINK_EXTRA})
endif(LINK_EXTRA)

# Build Type
# We need to make sure it's not empty
# Supported: Release, RelWithDebugInfo, Debug, Debugfull

# On WIN32, only Release seems to work correctly (?)
if(WIN32)
  set(DEFAULT_BUILD_TYPE "Release")
else(WIN32)
  set(DEFAULT_BUILD_TYPE "RelWithDebugInfo")
endif(WIN32)

set(CMAKE_BUILD_TYPE ${DEFAULT_BUILD_TYPE} CACHE STRING "CMake Build Type")
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE ${DEFAULT_BUILD_TYPE} CACHE STRING "CMake Build Type" FORCE)
endif(NOT CMAKE_BUILD_TYPE)

# Enable various flags on gcc
if(CMAKE_COMPILER_IS_GNUCXX)
  # Let's just hope that all gccs support these options and skip the tests...
  # -fno-strict-aliasing is needed apparently for Qt < 4.6
  set(CMAKE_CXX_FLAGS                  "${CMAKE_CXX_FLAGS} -ansi -Wall -Wextra -Wnon-virtual-dtor -fno-strict-aliasing")
  set(CMAKE_CXX_FLAGS_RELEASE          "-O2 ${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS_RELWITHDEBUGINFO "-g -O2 ${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS_DEBUG            "-g -ggdb -O0 -fno-reorder-blocks -fno-schedule-insns -fno-inline ${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS_DEBUGFULL        "-g3 ${CMAKE_CXX_FLAGS_DEBUG}")
  #SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb ")
endif(CMAKE_COMPILER_IS_GNUCXX)

string(TOUPPER ${CMAKE_BUILD_TYPE} upper_build_type)
if(upper_build_type STREQUAL "RELEASE" OR upper_build_type STREQUAL "RELWITHDEBUGINFO")
  add_definitions(-DNDEBUG -DQT_NO_DEBUG)
else(upper_build_type STREQUAL "RELEASE" OR upper_build_type STREQUAL "RELWITHDEBUGINFO")
  set(DEBUG 1)
endif(upper_build_type STREQUAL "RELEASE" OR upper_build_type STREQUAL "RELWITHDEBUGINFO")

if(APPLE AND DEPLOY)
  set(CMAKE_OSX_ARCHITECTURES "i386;ppc")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.4")
  set(CMAKE_OSX_SYSROOT "/Developer/SDKs/MacOSX10.4u.sdk/")
  add_definitions(-DMAC_10_4_SDK)
endif(APPLE AND DEPLOY)

# Dependencies
##############

# Execinfo is needed for generating backtraces
find_package(ExecInfo)
if(EXECINFO_FOUND)
  add_definitions(-DHAVE_EXECINFO)
  include_directories(${EXECINFO_INCLUDES})
  link_libraries(${EXECINFO_LIBRARIES})
endif(EXECINFO_FOUND)

# RPATH needs to be set correctly
# Do this down here, since otherwise KDE wants to handle it itself, and fails
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH 1)
set(CMAKE_BUILD_WITH_INSTALL_RPATH 1)

# Set global buildflags
# This is very much non-portable, so don't use -DSTATIC until you know what
# you do.
if(STATIC AND CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "-static-libgcc ${CMAKE_CXX_FLAGS}")
  link_directories(${CMAKE_BINARY_DIR}/staticlibs) # override dynamic libs
endif(STATIC AND CMAKE_COMPILER_IS_GNUCXX)

if(WIN32)
  link_libraries(imm32 winmm dbghelp Secur32)  # missing by default :/
  if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBUGINFO "/debug /INCREMENTAL:YES /NODEFAULTLIB:libcmt /DEFAULTLIB:msvcrt")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/debug /INCREMENTAL:YES /NODEFAULTLIB:libcmt")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUGFULL "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
    link_libraries(Version dwmapi shlwapi)
  endif(MSVC)
endif(WIN32)

if(NOT WIN32)
  check_function_exists(umask HAVE_UMASK)
  if(HAVE_UMASK)
    add_definitions(-DHAVE_UMASK)
  endif(HAVE_UMASK)
endif(NOT WIN32)

# origin configure style ...
# In this file we are doing all of our 'configure' checks. Things like checking
# for headers, functions, libraries, types and size of types.
INCLUDE (${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckTypeSize.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckCXXSourceCompiles.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckSymbolExists.cmake)

# To check for an include file you do this:
CHECK_INCLUDE_FILE("stdint.h"        HAVE_STDINT_H)

# To check the size of a primitive type:
CHECK_TYPE_SIZE("int"   SIZEOF_INT)

configure_file ("${PROJECT_SOURCE_DIR}/config-h.in"
                "${PROJECT_BINARY_DIR}/config.h" )

# Now make sure that you the the build directory on your "Include" path when compiling
include_directories(${PROJECT_BINARY_DIR})


#include_directories($(CMAKE_SOURCE_DIR)/include)
#include_directories($(CMAKE_SOURCE_DIR)/src)

# These variables will be added to the main targets (CORE, QTCLIENT, MONO)
#set(COMMON_DEPS ${RC_WIN32})
#set(CORE_DEPS )
#set(CLIENT_DEPS )

add_definitions(-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_REENTRANT)

# check packages.
#find_package(LibEvent)
#find_package(Thrift REQUIRED)
find_package(Boost COMPONENTS program_options python REQUIRED)
#FIND_PACKAGE( Boost 1.40 COMPONENTS program_options REQUIRED )
#INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

IF (Boost_FOUND)
    MESSAGE(STATUS "Found boost (lib) in: ${Boost_LIBRARY_DIRS}")
    SET(Boost_USE_STATIC_LIBS OFF)
    SET(Boost_USE_MULTITHREADED ON)
    SET(Boost_USE_STATIC_RUNTIME OFF)
    #reset options -> as we support Linux only, require boost does not the matter.
    find_package(Boost COMPONENTS program_options python REQUIRED)
ENDIF (Boost_FOUND)

FIND_PACKAGE(MsgPack REQUIRED)
FIND_PACKAGE(ZeroMQ)
IF (ZEROMQ_FOUND)
        MESSAGE(STATUS "Found zeromq (lib) in: ${ZEROMQ_LIBRARY}")
ENDIF (ZEROMQ_FOUND)

find_package(PythonInterp 2.7 REQUIRED)
#find_package(PythonLibs 2.7 REQUIRED)
find_package(PythonDev)
IF (PYTHONLIBS_FOUND)
     MESSAGE(STATUS "Found python lib in: ${PYTHON_LIBRARIES}")
ENDIF (PYTHONLIBS_FOUND)

# Add needed subdirs - the order is important, since src needs some vars set by other dirs
#add_subdirectory(data)
#add_subdirectory(icons)
#add_subdirectory(pics)
#add_subdirectory(po)
add_subdirectory(src)
add_subdirectory(python)
#add_subdirectory(examples)

#install (TARGETS mmseg DESTINATION lib)
#install (TARGETS mmseg_bin DESTINATION bin)


