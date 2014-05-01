# ==== unittest TFoo ====
#set(GTEST_ROOT ${PROJECT_SOURCE_DIR}/gmock-1.6.0/gtest)

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

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

SET(LIBMMSEG_SOURCE
    basedict.cpp
    segmentor.cpp
    seg_result.c
    utils/utf8_to_16.cpp
    utils/csr_utils.c
    utils/csr_mmap.c
)
#SET(LIBRARY_OUTPUT_PATH ../../lib/)
ADD_LIBRARY(mmseg_c STATIC ${LIBMMSEG_SOURCE} )

ADD_EXECUTABLE(mmseg mmseg_cli.cpp )
target_link_libraries(mmseg mmseg_c)

include_directories(${PROJECT_SOURCE_DIR}/third_party/glog/src)
ADD_EXECUTABLE(tst_cmake tst_cmake.cpp )
target_link_libraries(tst_cmake glog pthread)
#target_link_libraries(tst_tut )
ADD_EXECUTABLE(tst_dict tst_dict.cpp )

include_directories(${PROJECT_SOURCE_DIR}/third_party/luajit/src)
# FIXME: should be build_dir
include_directories(${PROJECT_BINARY_DIR}/third_party/luajit)

ADD_EXECUTABLE(tst_lua tst_lua.cpp )
target_link_libraries(tst_lua liblua)

#INSTALL(TARGETS rosetta DESTINATION ${LIBRARY_INSTALL_PATH})

