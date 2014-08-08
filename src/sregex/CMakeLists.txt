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


SET(LIBSREGEXLT_SOURCE
   sre_palloc.c
   sre_vm_bytecode.c  
   sre_yyparser.c  # generated code
   sre_capture.c  
   sre_regex.c           
   sre_vm_pike.c      
   sre_regex_compiler.c
)

#SET(LIBRARY_OUTPUT_PATH ../../lib/)
ADD_LIBRARY(sregex_lt STATIC ${LIBSREGEXLT_SOURCE} )

#ADD_EXECUTABLE(mmseg mmseg_cli.cpp )
#target_link_libraries(mmseg mmseg_c glog pthread gflags liblua)


#INSTALL(TARGETS rosetta DESTINATION ${LIBRARY_INSTALL_PATH})

