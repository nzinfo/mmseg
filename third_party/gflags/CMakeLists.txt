IF (WIN32)
  INCLUDE_DIRECTORIES( BEFORE
    ${PROJECT_SOURCE_DIR}/third_party/msinttypes
    ${PROJECT_SOURCE_DIR}/third_party/pthreads-w32/include
  )
ENDIF (WIN32)

IF (WIN32)
 add_definitions( -wd4996 )  
ENDIF (WIN32)

ADD_LIBRARY(gflags STATIC gflags.cc gflags_reporting.cc gflags_completions.cc)
TARGET_LINK_LIBRARIES(gflags pthread)
SET_TARGET_PROPERTIES(gflags PROPERTIES
         ARCHIVE_OUTPUT_DIRECTORY  ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

#LIBMV_INSTALL_THIRD_PARTY_LIB(gflags)
