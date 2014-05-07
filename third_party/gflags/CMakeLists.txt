IF(WIN32)
  INCLUDE_DIRECTORIES(
    ../msinttypes
    ../pthreads-w32/include
  )
ENDIF(WIN32)

IF (WIN32)
add_definitions( -wd4996 )  
ENDIF (WIN32)

ADD_LIBRARY(gflags gflags.cc gflags_reporting.cc gflags_completions.cc)
TARGET_LINK_LIBRARIES(gflags pthread)

#LIBMV_INSTALL_THIRD_PARTY_LIB(gflags)
