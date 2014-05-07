#include_directories(${PROJECT_SOURCE_DIR}/third_party)

#ADD_SUBDIRECTORY(glog)
#ADD_SUBDIRECTORY(gtest-1.7.0)
#ADD_SUBDIRECTORY(gflags)

ADD_SUBDIRECTORY(luajit)

IF (WIN32)
  ADD_SUBDIRECTORY(pthreads-w32)
ENDIF (WIN32)


#Installation macro for all third parties headers
#LIBMV_INSTALL_ALL_HEADERS()