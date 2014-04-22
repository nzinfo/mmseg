# ==== unittest TFoo ====
#set(GTEST_ROOT ${PROJECT_SOURCE_DIR}/gmock-1.6.0/gtest)

#include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)

IF(WIN32) # Check if we are on Windows
  if(MSVC) # Check if we are using the Visual Studio compiler
    include_directories(${PROJECT_SOURCE_DIR}/include/msvc)
  endif()
ENDIF()

#SET(LIBRARY_OUTPUT_PATH ../../lib/)
#ADD_LIBRARY(tst_cmake STATIC tst_cmake.cpp )
ADD_EXECUTABLE(tst_cmake tst_cmake.cpp )
#target_link_libraries(tst_tut )

#INSTALL(TARGETS rosetta DESTINATION ${LIBRARY_INSTALL_PATH})

