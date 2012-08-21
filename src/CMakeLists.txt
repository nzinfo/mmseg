
include_directories(${ZygiellaQt_SOURCE_DIR}/src )

include_directories( ${POSTGRESQL_INCLUDE_DIR}/server )
IF(WIN32)
  INCLUDE_DIRECTORIES(${POSTGRESQL_INCLUDE_DIR}/server/port/win32)
ENDIF(WIN32)

set(SOURCES
	utils/assert.c
	utils/csr_mmap.c
	utils/csr_utils.c
	utils/StringTokenizer.cpp
	utils/Utf8_16.cpp
    )
	
IF(WIN32)
	set(SOURCES
		${SOURCES}
		utils/bsd_getopt_win.c
		)
ELSE(WIN32)
	set(SOURCES
		${SOURCES}
		utils/bsd_getopt.c
		)	
ENDIF(WIN32)

set(HEADERS    
	config.win.h
	csr_typedefs.h
	utils/csr.h               
	utils/csr_assert.h
	utils/csr_mmap.h          
	utils/csr_pool.h          
	utils/csr_utils.h         
	utils/darts.h
	utils/freelist.h          
	utils/os.h                
	utils/scoped_ptr.h        
	utils/Singleton.h
	utils/StringTokenizer.h   
	utils/Utf8_16.h
	)

IF(WIN32)
	set(HEADERS
		${HEADERS}
		utils/bsd_getopt_win.h
		)
ELSE(WIN32)
	set(HEADERS
		${HEADERS}
		utils/bsd_getopt.h
		)	
ENDIF(WIN32)

set(PRIVATE_HEADERS
    include/ircencoder_p.h
    include/ircparser_p.h
    include/ircsession_p.h)
    
	
SET(LIBRARY_OUTPUT_PATH ../../lib/)
#ADD_LIBRARY(zygiella SHARED ${SOURCES} ${HEADERS} ${PRIVATE_HEADERS})
ADD_LIBRARY(mmseg STATIC ${SOURCES} ${HEADERS} )
#INSTALL(TARGETS zygiella DESTINATION ${LIBRARY_INSTALL_PATH})


