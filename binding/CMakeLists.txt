option ( PYTHON_BINDINGS "Build python bindings via swig" ON )
option ( JAVA_BINDINGS "Build Java bindings via swig" ON )
option ( LINK_PYTHON_LIBRARY "Link against python libraries" ON )

#------------------------------------------------------------------------------
# Strip Option

# Add option to strip wrapping libraries.
# Since the wrapping libraries don't get installed by the normal cmake
# installation process, this option enables stripping of the libraries
# as part of the build process. It should be used on the laguage
# targets and the the SimpleITK iterface, as those can be installed
# into the system.
option(MMSEG_BUILD_STRIP "Strip executables and libraries after building." OFF)
mark_as_advanced(MMSEG_BUILD_STRIP)
set(CMAKE_STRIP_FLAGS "-x" CACHE STRING "Flags used by strip in the post_build.")
mark_as_advanced(CMAKE_STRIP_FLAGS)
separate_arguments(CMAKE_STRIP_FLAGS)

function(mmseg_strip_target tgt)
  if(NOT MMSEG_BUILD_STRIP)
    return()
  endif()
  get_property(type TARGET ${tgt} PROPERTY TYPE)
  if(NOT type STREQUAL STATIC_LIBRARY)
    add_custom_command(
      TARGET ${tgt}
      POST_BUILD
      COMMAND ${CMAKE_STRIP} ${CMAKE_STRIP_FLAGS} "$<TARGET_FILE:${tgt}>"
      )
  endif()

endfunction()

#------------------------------------------------------------------------------
if ( PYTHON_BINDINGS )
  # workaround for cmake bug #0013449
  if ( NOT DEFINED CMAKE_FIND_ROOT_PATH )
    find_package ( SWIG )
  else ()
    find_program ( SWIG_EXECUTABLE NAMES swig2.0 swig )
    if ( SWIG_EXECUTABLE )
      set ( SWIG_USE_FILE ${CMAKE_ROOT}/Modules/UseSWIG.cmake )
      set ( SWIG_FOUND TRUE )
    endif ()
  endif ()
  #
  find_package ( PythonInterp )
  find_package ( PythonLibs )
endif ()

if ( SWIG_FOUND AND PYTHONINTERP_FOUND )
  include ( UseSWIG )
  
  #include ( UseSWIGLocal )

  set_source_files_properties ( mmseg.i PROPERTIES CPLUSPLUS ON )

  include_directories ( BEFORE ${CMAKE_SOURCE_DIR}/include )
  include_directories ( BEFORE ${CMAKE_SOURCE_DIR}/src )
  include_directories ( ${PYTHON_INCLUDE_DIRS} )
  link_directories ( ${CMAKE_CURRENT_BINARY_DIR}/../src )

  if ( DOCUMENTATION AND DOXYGEN_FOUND )
    set(CMAKE_SWIG_FLAGS -DDOXYGEN=${DOXYGEN_FOUND})
  endif()

  swig_add_module ( mmseg python mmseg.i )
  swig_link_libraries ( mmseg  mmseg_c)

  if ( LINK_PYTHON_LIBRARY )
    swig_link_libraries ( mmseg ${PYTHON_LIBRARIES} )
  endif ()
  #message( ${PYTHON_LIBRARIES} )

  set_target_properties ( ${SWIG_MODULE_mmseg_REAL_NAME} PROPERTIES NO_SONAME ON )

  execute_process ( COMMAND ${PYTHON_EXECUTABLE} -c "from distutils import sysconfig; print( sysconfig.get_python_lib( plat_specific=True, prefix='${CMAKE_INSTALL_PREFIX}' ) )"
                    OUTPUT_VARIABLE _ABS_PYTHON_MODULE_PATH
                    OUTPUT_STRIP_TRAILING_WHITESPACE )

  get_filename_component ( _ABS_PYTHON_MODULE_PATH ${_ABS_PYTHON_MODULE_PATH} ABSOLUTE )
  file ( RELATIVE_PATH _REL_PYTHON_MODULE_PATH ${CMAKE_INSTALL_PREFIX} ${_ABS_PYTHON_MODULE_PATH} )

  set ( PYTHON_MODULE_PATH
        ${_REL_PYTHON_MODULE_PATH}
      )

  install ( FILES ${CMAKE_CURRENT_BINARY_DIR}/ftdi1.py DESTINATION ${PYTHON_MODULE_PATH} )
  install ( TARGETS ${SWIG_MODULE_ftdi1_REAL_NAME} LIBRARY DESTINATION ${PYTHON_MODULE_PATH} )


  set ( LIBFTDI_PYTHON_MODULE_PATH ${CMAKE_INSTALL_PREFIX}/${PYTHON_MODULE_PATH} )
  set ( LIBFTDI_PYTHON_MODULE_PATH ${LIBFTDI_PYTHON_MODULE_PATH} PARENT_SCOPE ) # for ftdiconfig.cmake
  message(STATUS "Building python bindings via swig. Will be installed under ${LIBFTDI_PYTHON_MODULE_PATH}")

else ()
  message(STATUS "Not building python bindings")
endif ()

#
# JAVA SWIG configuration
#
if ( JAVA_BINDINGS )
  find_package ( Java REQUIRED )
  find_package ( JNI REQUIRED )
  include_directories ( ${JAVA_INCLUDE_PATH} ${JNI_INCLUDE_DIRS} )

  FIND_PROGRAM(Java_JAVADOC_EXECUTABLE
    NAMES javadoc
    HINTS ${_JAVA_HINTS}
    PATHS ${_JAVA_PATHS}
    )

  include_directories ( BEFORE ${CMAKE_SOURCE_DIR}/include )
  include_directories ( BEFORE ${CMAKE_SOURCE_DIR}/src )

  # Make sure the nested directory structure exists
  set(JAVA_SOURCE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/)
  set(JAVA_BINARY_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/build)
  file(MAKE_DIRECTORY ${JAVA_SOURCE_DIRECTORY})
  file(MAKE_DIRECTORY ${JAVA_BINARY_DIRECTORY})

  # Nicely write the bridge code in org/itk/simple
  set(CMAKE_SWIG_OUTDIR ${JAVA_SOURCE_DIRECTORY}/com/coreseek/mmseg)
  set(CMAKE_SWIG_FLAGS -package "com.coreseek.mmseg" ${CMAKE_SWIG_GLOBAL_FLAGS})
  set(SWIG_MODULE_MMSegJave_EXTRA_DEPS ${SWIG_EXTRA_DEPS}
    ${CMAKE_CURRENT_SOURCE_DIR}/Java.i )
  SWIG_add_module ( MMSegJava java mmseg.i mmsegJAVA_wrap.cxx)
  SWIG_link_libraries(MMSegJava ${MMSegJava_LIBRARIES} mmseg_c)
  set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/mmsegJAVA_wrap.cxx  PROPERTIES COMPILE_FLAGS "-w")
  #mmseg_strip_target( ${SWIG_MODULE_MMSegJava_TARGET_NAME} )

  # Add target for org.itk.simple.jar
  add_custom_target(com_coreseek_mmseg ALL DEPENDS ${JAR_FILE})
  set(JAVA_SOURCE_CODE ${JAVA_SOURCE_DIRECTORY}/com/coreseek/mmseg/*.java)

  # Add custom command and target to compile the generated files and put them in a jar file
  # Make sure the commands depend on the output library from SWIG
 

  # Get the location of the extension directory
  string(REGEX REPLACE "include"  "jre/lib/ext" JAVA_EXTENSION_DIR ${JAVA_INCLUDE_PATH} )

  # TODO: add new target to install simpleitk java
  # Add the install target
  # install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${JAR_FILE} DESTINATION ${JAVA_EXTENSION_DIR})

endif()
