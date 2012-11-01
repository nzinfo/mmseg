# Builds the core module

set(SOURCES
    css/mmthunk.cpp           
    css/SegmentPkg.cpp     
    css/UnigramCorpusReader.cpp
    css/segmenter.cpp         
    css/SynonymsDict.cpp   
    css/UnigramDict.cpp
    css/SegmenterManager.cpp  
    css/ThesaurusDict.cpp  
    css/UnigramRecord.cpp
    
    iniparser/dictionary.c  
    iniparser/iniparser.c
    
    utils/assert.c      
#    utils/bsd_getopt_win.c  
    utils/csr_utils.c
    utils/bsd_getopt.c  
    utils/csr_mmap.c
    
    utils/StringTokenizer.cpp  
    utils/Utf8_16.cpp
    )

set(HEADERS
    css
    iniparser
    utils
    )

    
#set(CORE_RCS ${CORE_RCS} core/sql.qrc PARENT_SCOPE)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/css)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/iniparser)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/utils)

#add_custom_target(GenerateMoc
#  DEPENDS ${SOURCE_MOC}
#)

#add_definitions(-DCOMMUNI_STATIC)
add_library(mmseg STATIC ${SOURCES} ${HEADERS})

SET_TARGET_PROPERTIES( mmseg PROPERTIES COMPILE_FLAGS -fPIC)

#add_dependencies(irc_core GenerateMoc)
#add_dependencies(irc_core mod_common)

# build the demo app
add_executable(mmseg_bin mmseg_main.cpp )
set_target_properties(mmseg_bin PROPERTIES
                               OUTPUT_NAME ./mmseg)
target_link_libraries(mmseg_bin mmseg)
add_dependencies(mmseg_bin mmseg)

add_executable(tst_stdin tests/tst_timeout_std.c )

install (TARGETS mmseg DESTINATION lib)
install (TARGETS mmseg_bin DESTINATION bin)

install (FILES csr_typedefs.h DESTINATION include/mmseg)

install (FILES css/ICorpusReader.h DESTINATION include/mmseg)
install (FILES css/Segmenter.h  DESTINATION include/mmseg)
install (FILES css/SegmentPkg.h  DESTINATION include/mmseg)
install (FILES css/ThesaurusDict.h DESTINATION include/mmseg)
install (FILES css/UnigramCorpusReader.h DESTINATION include/mmseg)
install (FILES css/UnigramRecord.h DESTINATION include/mmseg)
install (FILES css/SegmenterManager.h DESTINATION include/mmseg)
install (FILES css/SynonymsDict.h  DESTINATION include/mmseg)
install (FILES css/mmthunk.h  DESTINATION include/mmseg)
install (FILES css/tolowercase.h DESTINATION include/mmseg)
install (FILES css/UnigramDict.h DESTINATION include/mmseg)

install (FILES utils/bsd_getopt.h DESTINATION include/mmseg)
install (FILES utils/csr.h   DESTINATION include/mmseg)
install (FILES utils/csr_utils.h DESTINATION include/mmseg)
install (FILES utils/os.h  DESTINATION include/mmseg)
install (FILES utils/StringTokenizer.h DESTINATION include/mmseg)
install (FILES utils/csr_mmap.h  DESTINATION include/mmseg)
install (FILES utils/darts.h  DESTINATION include/mmseg)
install (FILES utils/scoped_ptr.h DESTINATION include/mmseg)
install (FILES utils/Utf8_16.h DESTINATION include/mmseg)
install (FILES utils/csr_assert.h  DESTINATION include/mmseg)
install (FILES utils/csr_pool.h DESTINATION include/mmseg)
install (FILES utils/freelist.h DESTINATION include/mmseg)
install (FILES utils/Singleton.h DESTINATION include/mmseg)
                     
install (FILES iniparser/dictionary.h  DESTINATION include/mmseg)
install (FILES iniparser/iniparser.h DESTINATION include/mmseg)


                  
              
