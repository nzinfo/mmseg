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

#set(PRIVATE_HEADERS
#    include/ircencoder_p.h
#    include/ircparser_p.h
#    include/ircsession_p.h)
    
#qt4_wrap_cpp(MOC ${MOC_HDRS})
#QT4_GENERATE_MOC(${CMAKE_CURRENT_SOURCE_DIR}/include/irc.h  
#    ${CMAKE_CURRENT_BINARY_DIR}/include/moc_irc.cxx)
#QT4_GENERATE_MOC(${CMAKE_CURRENT_SOURCE_DIR}/include/irccommand.h 
#    ${CMAKE_CURRENT_BINARY_DIR}/include/moc_irccommand.cxx)
#QT4_GENERATE_MOC(${CMAKE_CURRENT_SOURCE_DIR}/include/ircmessage.h 
#    ${CMAKE_CURRENT_BINARY_DIR}/include/moc_ircmessage.cxx)

#QT4_GENERATE_MOC(${CMAKE_CURRENT_SOURCE_DIR}/include/ircsession.h 
#    ${CMAKE_CURRENT_BINARY_DIR}/include/moc_ircsession.cxx)

#qt4_wrap_cpp(MOC ${MOC_SESSION} OPTIONS -fircsession_p.h )


#set(
#  HEADERS_ONLY_MOC
#  ${CMAKE_CURRENT_BINARY_DIR}/include/moc_ircsession.cxx
#)

#set(
#  HEADERS_MOC
#  ${CMAKE_CURRENT_BINARY_DIR}/include/moc_irc.cxx
#  ${CMAKE_CURRENT_BINARY_DIR}/include/moc_irccommand.cxx
#  ${CMAKE_CURRENT_BINARY_DIR}/include/moc_ircmessage.cxx
#)

#set_source_files_properties(
#  ${HEADERS_ONLY_MOC}
#  PROPERTIES
#    HEADER_FILE_ONLY true
#)

#set(SOURCES ${SOURCE_MOC} ${SOURCES} )
    
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

