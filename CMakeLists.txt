CMAKE_MINIMUM_REQUIRED(VERSION 2.3)
PROJECT(PGMMSEG)

#-----------------------------------------------------------------------------
# PGMMSEG version number.
SET(PGMMSEG_VERSION_MAJOR "1")
SET(PGMMSEG_VERSION_MINOR "0")
SET(PGMMSEG_VERSION_PATCH "5")

SET(PGMMSEG_VERSION_STRING "${PGMMSEG_VERSION_MAJOR}.${PGMMSEG_VERSION_MINOR}.${PGMMSEG_VERSION_PATCH}")

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

include (MacroLibrary)

macro_optional_find_package(PostgreSQL)
IF(NOT POSTGRESQL_FOUND)
  MESSAGE(FATAL ERROR " Please check your PostgreSQL installation.") 
ENDIF(NOT POSTGRESQL_FOUND)

#FIND_PACKAGE(Boost)
#IF(Boost_INCLUDE_DIRS)
#  MESSAGE("Boost headers were found here: ${Boost_INCLUDE_DIR}")
#ELSE(Boost_INCLUDE_DIRS)
#  MESSAGE(FATAL ERROR " Please check your Boost installation ") 
#ENDIF(Boost_INCLUDE_DIRS)

EXEC_PROGRAM(pg_config
 ARGS --pkglibdir
 OUTPUT_VARIABLE LIB_DIR)

IF(LIB_DIR)
  MESSAGE("Output directory for libraries is set to ${LIB_DIR}")
ELSE(LIB_DIR)
  SET(LIB_DIR ${PGMMSEG_BINARY_DIR}/lib)
  MESSAGE("pg_config not found, output directory for libraries is set to ${ZYGEXT_BINARY_DIR}/lib")
ENDIF(LIB_DIR)

IF(UNIX)
  SET(LIBRARY_INSTALL_PATH ${LIB_DIR})
  SET(SQL_INSTALL_PATH /usr/share/postlbs)
  MESSAGE("Installation directory for libraries is set to ${LIBRARY_INSTALL_PATH} and for SQL files is set to ${SQL_INSTALL_PATH}")
ELSE(UNIX)
  SET(LIBRARY_INSTALL_PATH ${PGMMSEG_BINARY_DIR}/lib)
  SET(SQL_INSTALL_PATH ${PGMMSEG_BINARY_DIR}/lib)
ENDIF(UNIX)

SET(PGMMSEG_INCLUDE_DIRECTORIES ${PGMMSEG_SOURCE_DIR}/src ${PGMMSEG_SOURCE_DIR}/src/css )

INCLUDE_DIRECTORIES(. ${PGMMSEG_INCLUDE_DIRECTORIES} ${POSTGRESQL_INCLUDE_DIR} )
IF(WIN32)
  INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES} ${POSTGRESQL_INCLUDE_DIR}/port/win32)
ENDIF(WIN32)

# we need link libpostgres.a on win32 env
IF(WIN32)
  LINK_DIRECTORIES(${LIB_DIR})
  LINK_LIBRARIES(postgres)
ENDIF(WIN32)

SET(CMAKE_C_FLAGS "-O2 -g -frounding-math")
SET(CMAKE_CXX_FLAGS "-O2 -g -frounding-math")

# Recurse into the subdirectories.  This does not actually
# cause another cmake executable to run.  The same process will walk through
# the project's entire directory structure.
SUBDIRS(src)


