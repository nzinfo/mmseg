
INCLUDE_DIRECTORIES("${Boost_INCLUDE_DIRS}" "${PYTHON_INCLUDE_DIRS}")

FIND_PACKAGE(Boost COMPONENTS python)
ADD_LIBRARY(pymmseg SHARED pymmseg.cpp)
TARGET_LINK_LIBRARIES(pymmseg ${Boost_LIBRARIES})
 
IF(CMAKE_COMPILER_IS_GNUCXX)
   ADD_DEFINITIONS("-Wall")
ELSE()
   MESSAGE(FATAL_ERROR "CMakeLists.txt has not been tested/written for your compiler.")
ENDIF()
