# ==== unittest TFoo ====
#set(GTEST_ROOT ${PROJECT_SOURCE_DIR}/gmock-1.6.0/gtest)

#FIXME: How to reuse this block?
IF (WIN32)
  INCLUDE_DIRECTORIES( BEFORE
    ${PROJECT_SOURCE_DIR}/third_party/glog/src/windows
     ${PROJECT_SOURCE_DIR}/third_party/msinttypes
  )
ELSE (WIN32)
  #INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/third_party/glog/src/posix)
  IF(APPLE)
    INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/third_party/glog/src/mac)
  ENDIF(APPLE)
  
  IF(UNIX AND NOT APPLE)
    INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/third_party/glog/src/unix)
  ENDIF(UNIX AND NOT APPLE)

ENDIF (WIN32)

# FIX LUAJIT @MAC
IF(APPLE)
    SET(CMAKE_EXE_LINKER_FLAGS "-pagezero_size 10000 -image_base 100000000 ${CMAKE_EXE_LINKER_FLAGS}")
ENDIF(APPLE)

#include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/src/utils)

IF(WIN32) # Check if we are on Windows
  if(MSVC) # Check if we are using the Visual Studio compiler
    include_directories(${PROJECT_SOURCE_DIR}/include/msvc)
  endif()
ENDIF()

IF (WIN32)
    add_definitions(-DWIN32)    # how about x64?
ELSE (WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
ENDIF (WIN32)


SET(LIBMMSEG_SOURCE
    mm_charmap.cc
    mm_stringpool.cc
    mm_entry_datapool.cc
    mm_entrydata.cc
    mm_match_result.cc
    mm_dict_base.cc
    mm_dict_mgr.cc
    mm_dict_updatable.cc
    utils/utf8_to_16.cpp
    utils/csr_utils.c
    utils/csr_mmap.c
)
#SET(LIBRARY_OUTPUT_PATH ../../lib/)
ADD_LIBRARY(mmseg_c STATIC ${LIBMMSEG_SOURCE} )

ADD_EXECUTABLE(mmseg mmseg_cli.cpp )
target_link_libraries(mmseg mmseg_c glog pthread gflags)

include_directories(${PROJECT_SOURCE_DIR}/third_party/)
include_directories(${PROJECT_SOURCE_DIR}/third_party/glog/src)
include_directories(${PROJECT_SOURCE_DIR}/third_party/gtest-1.7.0/include)
ADD_EXECUTABLE(tst_cmake tst_cmake.cpp )
target_link_libraries(tst_cmake glog pthread gflags)
#target_link_libraries(tst_tut )

# the following use gtest
IF (WIN32)
  add_definitions(-DGTEST_HAS_TR1_TUPLE=0)
ENDIF (WIN32)

ADD_EXECUTABLE(tst_dict tst_dict.cpp )
target_link_libraries(tst_dict mmseg_c glog pthread gflags gtest)

include_directories(${PROJECT_SOURCE_DIR}/third_party/luajit/src)
# FIXME: should be build_dir
include_directories(${PROJECT_BINARY_DIR}/third_party/luajit)

ADD_EXECUTABLE(tst_lua tst_lua.cpp )
target_link_libraries(tst_lua liblua)

#INSTALL(TARGETS rosetta DESTINATION ${LIBRARY_INSTALL_PATH})

