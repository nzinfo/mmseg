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

set(SOURCES
	${SOURCES}
	css/mmthunk.cpp               
	css/segmenter.cpp             
	css/SegmenterManager.cpp
	css/SegmentPkg.cpp            
	css/SynonymsDict.cpp          
	css/ThesaurusDict.cpp
	css/UnigramCorpusReader.cpp   
	css/UnigramDict.cpp           
	css/UnigramRecord.cpp
	iniparser/iniparser.c
	iniparser/dictionary.c 
    )

IF(WIN32)
	set(SOURCES
		${SOURCES}
		#utils/bsd_getopt_win.c
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
	iniparser/dictionary.h   
	iniparser/iniparser.h
	)

IF(WIN32)
	set(HEADERS
		${HEADERS}
		#utils/bsd_getopt_win.h
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
	
set(MMSEGCLI_SOURCES
    mmseg_main.cpp
    )

set(PGMMSEG_SOURCES
    pgmmseg.c
    )
	
SET(LIBRARY_OUTPUT_PATH ../../lib/)
#ADD_LIBRARY(zygiella SHARED ${SOURCES} ${HEADERS} ${PRIVATE_HEADERS})
ADD_LIBRARY(mmseg STATIC ${SOURCES} ${HEADERS} )
#INSTALL(TARGETS zygiella DESTINATION ${LIBRARY_INSTALL_PATH})
 
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

MESSAGE("Output directory for libraries is set to ${POSTGRESQL_INCLUDE_DIR}")

ADD_LIBRARY(pgmmseg SHARED ${PGMMSEG_SOURCES} )
target_link_libraries(pgmmseg mmseg )
add_dependencies(pgmmseg mmseg)

SET(LIBRARY_INSTALL_PATH ${POSTGRESQL_EXTLIB_DIR} )
INSTALL(TARGETS pgmmseg DESTINATION ${LIBRARY_INSTALL_PATH})

#add_executable(mmseg_cli ${MMSEGCLI_SOURCES}  )
#target_link_libraries(mmseg_cli mmseg )
#add_dependencies(mmseg_cli mmseg)
