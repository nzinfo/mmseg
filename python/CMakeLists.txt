
INCLUDE_DIRECTORIES("${Boost_INCLUDE_DIRS}" "${PYTHON_INCLUDE_DIR}")

set(MAIN_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)

include_directories(${MAIN_SOURCE_DIR})
include_directories(${MAIN_SOURCE_DIR}/css)
include_directories(${MAIN_SOURCE_DIR}/iniparser)
include_directories(${MAIN_SOURCE_DIR}/utils)


FIND_PACKAGE(Boost COMPONENTS python)
ADD_LIBRARY(pymmseg SHARED pymmseg.cpp)
#set_target_properties(pymmseg PROPERTIES
#                               OUTPUT_NAME pymmseg)
set_target_properties(pymmseg PROPERTIES PREFIX "")

TARGET_LINK_LIBRARIES(pymmseg ${Boost_LIBRARIES} ${PYTHON_LIBRARIES})
 
IF(CMAKE_COMPILER_IS_GNUCXX)
   ADD_DEFINITIONS("-Wall")
ELSE()
   MESSAGE(FATAL_ERROR "CMakeLists.txt has not been tested/written for your compiler.")
ENDIF()
