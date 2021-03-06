# - Find LZ4 (lz4.h, liblz4.a, liblz4.so, and liblz4.so.1)
# This module defines
#  LZ4_INCLUDE_DIRS, directory containing headers
#  LZ4_LIBRARY_DIRS, directory containing lz4 libraries
#  LZ4_LIBRARY, absolute path to lz4 library
#  LZ4_FOUND, whether lz4 has been found
#  LZ4_USE_STATIC_LIBS, whether lz4 has been found (default: OFF)

set(_LZ4_ROOT $ENV{LZ4_ROOT})

if(NOT LZ4_USE_STATIC_LIBS)
  set(LZ4_USE_STATIC_LIBS OFF)
endif()

#Check whether to search static or dynamic libs
set( CMAKE_FIND_LIBRARY_SUFFIXES_SAV ${CMAKE_FIND_LIBRARY_SUFFIXES} )

if( ${LZ4_USE_STATIC_LIBS} OR NOT ${BUILD_SHARED_LIBS})
  if (NOT LZ4_FIND_QUIETLY)
    message("** [FindLZ4] searching for static lib")
  endif ()
  set(LZ4_SEARCH_NAMES liblz4_static${CMAKE_STATIC_LIBRARY_SUFFIX} lz4${CMAKE_STATIC_LIBRARY_SUFFIX} liblz4${CMAKE_STATIC_LIBRARY_SUFFIX})
else()
  #set(LZ4_SEARCH_NAMES lz4${CMAKE_SHARED_LIBRARY_SUFFIX} liblz4${CMAKE_SHARED_LIBRARY_SUFFIX})
  set(LZ4_SEARCH_NAMES lz4 liblz4)
endif()

if (NOT LZ4_FIND_QUIETLY)
  message("** [FindLZ4] search for ${LZ4_SEARCH_NAMES}")
endif ()


if(IS_DIRECTORY ${LZ4_ROOT})

  
  IF(WIN32) #assumes MSVC
    find_library(LZ4_LIB_PATH NAMES ${LZ4_SEARCH_NAMES} NAMES_PER_DIR
      HINTS ${LZ4_ROOT}
      PATHS ${LZ4_ROOT}
      PATH_SUFFIXES lib bin
      NO_DEFAULT_PATH)
    # IF(NOT LZ4_LIB_PATH)
    #   FILE(GLOB REC_PATHS ${LZ4_ROOT}/bin/liblz4* ${LZ4_ROOT}/liblz4* ${LZ4_ROOT}/lib/liblz4* ${LZ4_ROOT}/lib/lz4*)
    #   FOREACH(FPATH ${REC_PATHS})
    # 	IF(${FPATH} MATCHES ".*lz4.dll")
    # 	  SET(LZ4_LIB_PATH ${FPATH})
    # 	ENDIF()
    # 	IF(${FPATH} MATCHES ".*lz4.lib")
    # 	  SET(LZ4_LIB_PATH ${FPATH})
    # 	ENDIF()
    # 	IF(${FPATH} MATCHES ".*lz4.a")
    # 	  SET(LZ4_LIB_PATH ${FPATH})
    # 	ENDIF()
    #   ENDFOREACH(FPATH)
      
    # ENDIF()
    find_path(LZ4_INC_PATH lz4.h HINTS ${LZ4_ROOT} PATHS ${LZ4_ROOT} PATH_SUFFIXES inc include NO_DEFAULT_PATH)
    IF(NOT LZ4_INC_PATH)
      FILE(GLOB REC_PATHS ${LZ4_ROOT}/lz4*.h ${LZ4_ROOT}/include/lz4*.h ${LZ4_ROOT}/inc/lz4*.h)
      FOREACH(FPATH ${REC_PATHS})
	IF(${FPATH} MATCHES ".*lz4.h")
	  SET(LZ4_INC_PATH ${FPATH})
	ENDIF()
	
      ENDFOREACH(FPATH)
    ENDIF()
  ELSE(WIN32)
    
    
    find_library(LZ4_LIB_PATH
      NAMES ${LZ4_SEARCH_NAMES} NAMES_PER_DIR
      HINTS ${LZ4_ROOT}
      PATHS ${LZ4_ROOT}
      PATH_SUFFIXES lib lib64 bin
      NO_DEFAULT_PATH)
    
    get_filename_component(LZ4_LIB_PATHONLY ${LZ4_LIB_PATH} DIRECTORY)
    
    find_path(LZ4_INC_PATH lz4.h HINTS ${LZ4_ROOT} ${LZ4_LIB_PATHONLY}/.. PATHS ${LZ4_ROOT} ${LZ4_LIB_PATHONL}/.. PATH_SUFFIXES lib inc include NO_DEFAULT_PATH)
    
  ENDIF(WIN32)
  
  if (NOT LZ4_FIND_QUIETLY)
    message("** [FindLZ4] Found: ${LZ4_LIB_PATH} ${LZ4_INC_PATH} with LZ4_ROOT = ${LZ4_ROOT}")
  endif ()
  
  
else(IS_DIRECTORY ${LZ4_ROOT})    

  find_library(LZ4_LIB_PATH NAMES ${LZ4_SEARCH_NAMES} NAMES_PER_DIR PATH_SUFFIXES lib lib64)
  find_library(LZ4_LIB_PATH NAMES ${LZ4_SEARCH_NAMES} NAMES_PER_DIR PATH_SUFFIXES include inc bin)
  
  find_path(LZ4_INC_PATH lz4.h PATH_SUFFIXES lib lib64 inc include bin)
  
  if (NOT LZ4_FIND_QUIETLY)
    message( "** [FindLZ4] Found: ${LZ4_LIB_PATH} ${LZ4_INC_PATH} without LZ4_ROOT")
  endif ()

  if(EXISTS ${LZ4_INC_PATH})
    if(NOT EXISTS ${LZ4_LIB_PATH})
      get_filename_component(ROOT_SEARCH_PATH ${LZ4_INC_PATH} DIRECTORY)
      unset(LZ4_LIB_PATH)
      unset(LZ4_LIB_PATH-NOTFOUND)
      message( "** [FindLZ4] trying ${ROOT_SEARCH_PATH}/{lib lib64 bin}")
      find_library(LAST_CHANCE_PATH NAMES ${LZ4_SEARCH_NAMES} NAMES_PER_DIR
	PATHS ${ROOT_SEARCH_PATH}
	PATH_SUFFIXES lib lib64 inc include bin
	HINTS ${ROOT_SEARCH_PATH}
	)
      message( "** [FindLZ4] last chance ${LAST_CHANCE_PATH}")
      if(EXISTS ${LAST_CHANCE_PATH})
	set(LZ4_LIB_PATH ${LAST_CHANCE_PATH})
      endif()
    endif()
  endif()
  
endif(IS_DIRECTORY ${LZ4_ROOT})


if (LZ4_INC_PATH AND LZ4_LIB_PATH)
  set(LZ4_FOUND TRUE)
  #set(LZ4_LIBRARY_DIRS ${LZ4_LIB_PATH})
  if(${CMAKE_VERSION} VERSION_GREATER "2.8.11")
    get_filename_component(LZ4_LIBRARY_DIRS ${LZ4_LIB_PATH} DIRECTORY)
  else()
    get_filename_component(LZ4_LIBRARY_DIRS ${LZ4_LIB_PATH} PATH)
  endif()
  set(LZ4_INCLUDE_DIRS ${LZ4_INC_PATH})
  set(LZ4_LIBRARY ${LZ4_LIB_PATH})
  include_directories(${LZ4_INC_PATH})
  if(IS_DIRECTORY ${LZ4_LIB_PATH})
    link_directories(${LZ4_LIB_PATH})
  else()
    get_filename_component(LZ4_LIB_ABSPATH ${LZ4_LIB_PATH} ABSOLUTE)
    get_filename_component(LZ4_LIB_DIRNAME ${LZ4_LIB_ABSPATH} DIRECTORY)
    link_directories(${LZ4_LIB_DIRNAME})
  endif()

  if(${LZ4_LIB_PATH} MATCHES ".*${CMAKE_STATIC_LIBRARY_SUFFIX}")
    add_library(lz4 STATIC IMPORTED)
  else()
    add_library(lz4 SHARED IMPORTED)
  endif()

  set_target_properties(lz4 PROPERTIES LINKER_LANGUAGE C)
  set_target_properties(lz4 PROPERTIES IMPORTED_LOCATION ${LZ4_LIBRARY})
else ()

  set(LZ4_FOUND FALSE)
  unset(lz4)
  
endif ()

#revert CMAKE_FIND_LIBRARY_SUFFIXES
set( CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES_SAV} )


mark_as_advanced(LZ4_LIBRARY LZ4_LIBRARY_DIRS LZ4_INCLUDE_DIRS LZ4_STATIC_LIB LZ4_SHARED_LIB)
