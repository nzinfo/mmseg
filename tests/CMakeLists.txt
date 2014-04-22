# ==== unittest TFoo ====
#set(GTEST_ROOT ${PROJECT_SOURCE_DIR}/gmock-1.6.0/gtest)

IF(APPLE)
    SET(CMAKE_EXE_LINKER_FLAGS "-pagezero_size 10000 -image_base 100000000 ${CMAKE_EXE_LINKER_FLAGS}")
ENDIF(APPLE)

#include_directories(${PROJECT_SOURCE_DIR})
#include_directories(${PROJECT_SOURCE_DIR}/librosetta/include)
#include_directories(${PROJECT_SOURCE_DIR}/librosetta/deps/luajit/src)

IF(WIN32) # Check if we are on Windows
  if(MSVC) # Check if we are using the Visual Studio compiler
    include_directories(${PROJECT_SOURCE_DIR}/librosetta/include/msvc)
  endif()
ENDIF()


ADD_EXECUTABLE(tst_gtest tst_gtest.cpp )
#target_link_libraries(tst_tut )
target_link_libraries(tst_gtest gtest gtest_main )


