# - Find LZ4 (lz4.h, liblz4.a, liblz4.so, and liblz4.so.1)
# This module defines
#  LZ4_INCLUDE_DIRS, directory containing headers
#  LZ4_LIBRARY_DIRS, directory containing lz4 libraries
#  LZ4_LIBRARY, absolute path to lz4 library
#  LZ4_FOUND, whether lz4 has been found
#  LZ4_USE_STATIC_LIBS, whether lz4 has been found (default: OFF)

set(_LZ4_ROOT $ENV{LZ4_ROOT})

option(LZ4_USE_STATIC_LIBS "whether to search for static libs or not" OFF)

#Check whether to search static or dynamic libs
set( CMAKE_FIND_LIBRARY_SUFFIXES_SAV ${CMAKE_FIND_LIBRARY_SUFFIXES} )

if( ${LZ4_USE_STATIC_LIBS} )
  set( CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_STATIC_LIBRARY_SUFFIX} )
else()
  set( CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_SHARED_LIBRARY_SUFFIX} )
endif()


if(IS_DIRECTORY ${LZ4_ROOT})

  
  IF(WIN32) #assumes MSVC
    find_library(LZ4_LIB_PATH NAMES lz4 liblz4 # lz4.dll liblz4.lib lz4.lib liblz4.dll
      NAMES_PER_DIR HINTS ${LZ4_ROOT} PATHS ${LZ4_ROOT} PATH_SUFFIXES lib bin NO_DEFAULT_PATH)
    IF(NOT LZ4_LIB_PATH)
      FILE(GLOB REC_PATHS ${LZ4_ROOT}/bin/liblz4* ${LZ4_ROOT}/liblz4* ${LZ4_ROOT}/lib/liblz4* ${LZ4_ROOT}/lib/lz4*)
      FOREACH(FPATH ${REC_PATHS})
	IF(${FPATH} MATCHES ".*lz4.dll")
	  SET(LZ4_LIB_PATH ${FPATH})
	ENDIF()
	IF(${FPATH} MATCHES ".*lz4.lib")
	  SET(LZ4_LIB_PATH ${FPATH})
	ENDIF()
	IF(${FPATH} MATCHES ".*lz4.a")
	  SET(LZ4_LIB_PATH ${FPATH})
	ENDIF()
      ENDFOREACH(FPATH)
      
    ENDIF()
    find_path(LZ4_INC_PATH lz4.h HINTS ${LZ4_ROOT} PATHS ${LZ4_ROOT} PATH_SUFFIXES inc include NO_DEFAULT_PATH)
    IF(NOT LZ4_INC_PATH)
      FILE(GLOB REC_PATHS ${LZ4_ROOT}/lz4*.h ${LZ4_ROOT}/include/lz4*.h ${LZ4_ROOT}/inc/lz4*.h)
      FOREACH(FPATH ${REC_PATHS})
	IF(${FPATH} MATCHES ".*lz4.h")
	  SET(LZ4_INC_PATH ${FPATH})
	ENDIF()
	
      ENDFOREACH(FPATH)
    ENDIF()
  ELSE()
    find_path(LZ4_APP_PATH lz4)

    find_library(LZ4_LIB_PATH NAMES lz4 liblz4 # liblz4.dylib liblz4.a lz4.a
      NAMES_PER_DIR HINTS ${LZ4_APP_PATH}/.. ${LZ4_ROOT} PATHS ${LZ4_APP_PATH}/.. ${LZ4_ROOT} PATH_SUFFIXES lib lib64 bin NO_DEFAULT_PATH)

    find_path(LZ4_INC_PATH lz4.h HINTS ${LZ4_ROOT} ${LZ4_APP_PATH}/.. PATHS ${LZ4_ROOT} ${LZ4_APP_PATH}/.. PATH_SUFFIXES lib inc include NO_DEFAULT_PATH)
  ENDIF()
  
  if (NOT LZ4_FIND_QUIETLY)
    message("** [FindLZ4] Found: ${LZ4_LIB_PATH} ${LZ4_INC_PATH} with LZ4_ROOT = ${LZ4_ROOT}")
  endif ()
  
  
else()    

  find_path(LZ4_APP_PATH lz4)
  find_library(LZ4_LIB_PATH NAMES lz4 liblz4 liblz4.dylib liblz4.a lz4.a NAMES_PER_DIR HINTS ${LZ4_APP_PATH}/.. PATHS ${LZ4_APP_PATH}/.. PATH_SUFFIXES lib lib64 bin )
  find_path(LZ4_INC_PATH lz4.h HINTS ${LZ4_APP_PATH}/.. PATHS ${LZ4_APP_PATH}/.. PATH_SUFFIXES lib inc include)
    
  # find_path(LZ4_INC_PATH lz4.h)
  # find_library(LZ4_LIB_PATH NAMES lz4)
  if (NOT LZ4_FIND_QUIETLY)
    message( "** [FindLZ4] Found: ${LZ4_LIB_PATH} ${LZ4_INC_PATH} without LZ4_ROOT")
  endif ()

endif()


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
  
else ()

  set(LZ4_FOUND FALSE)
  
endif ()

# if (NOT LZ4_FIND_QUIETLY)
#     message( "** [FindLZ4] searching ${LZ4_LIBRARY_DIRS} for libraries")
#   endif ()
  
# if (LZ4_FOUND)
# 	if(UNIX)

# 		if (APPLE)
# 			if (EXISTS ${LZ4_LIBRARY_DIRS}/liblz4.dylib)	       		       
# 				set(LZ4_SHARED_LIB ${LZ4_LIBRARY_DIRS}/liblz4.dylib)
# 			endif()
# 		else(APPLE)
			
# 			if (EXISTS ${LZ4_LIBRARY_DIRS}/liblz4.so)	       		       
# 				set(LZ4_SHARED_LIB ${LZ4_LIBRARY_DIRS}/liblz4.so)
# 			endif()
# 		endif(APPLE)
  
# 	ELSE(UNIX)
# 		IF(WIN32)
# 			if (EXISTS ${LZ4_LIBRARY_DIRS}/liblz4_static.lib)
# 				set(LZ4_STATIC_LIB ${LZ4_LIBRARY_DIRS}/liblz4_static.lib)
# 			endif()
# 			if (EXISTS ${LZ4_LIBRARY_DIRS}/liblz4.a)
# 				set(LZ4_STATIC_LIB ${LZ4_LIBRARY_DIRS}/liblz4.a)
# 			endif()
# 			if (EXISTS ${LZ4_LIBRARY_DIRS}/liblz4.lib)
# 				set(LZ4_STATIC_LIB ${LZ4_LIBRARY_DIRS}/liblz4.lib)
# 			endif()
# 			if (EXISTS ${LZ4_LIBRARY_DIRS}/lz4.lib)
# 				set(LZ4_STATIC_LIB ${LZ4_LIBRARY_DIRS}/lz4.lib)
# 			endif()
# 			if (EXISTS ${LZ4_LIBRARY_DIRS}/liblz4.dll)
# 				set(LZ4_SHARED_LIB ${LZ4_LIBRARY_DIRS}/liblz4.dll)
# 			endif()
# 			#honoring msys2
# 			if (EXISTS ${LZ4_LIBRARY_DIRS}/../bin/liblz4.dll)
# 				set(LZ4_SHARED_LIB ${LZ4_LIBRARY_DIRS}/../bin/liblz4.dll)
# 			endif()
# 		ENDIF(WIN32)
# 	ENDIF(UNIX)
#   if (NOT LZ4_FIND_QUIETLY)
#     message( "** Found LZ4 library: ${LZ4_LIBRARY_DIRS} ${LZ4_INCLUDE_DIRS} \n(static lib: ${LZ4_STATIC_LIB}), dyn lib: ${LZ4_SHARED_LIB}")
#   endif ()
# else ()
#   if (NOT LZ4_FIND_QUIETLY)
#       message(WARNING "** LZ4_LIB_PATH ${LZ4_LIB_PATH}")
#       message(WARNING "** LZ4_INC_PATH ${LZ4_INC_PATH}")
# 	  message(FATAL_ERROR "** LZ4 not found **")
#   endif ()
# endif ()

#revert CMAKE_FIND_LIBRARY_SUFFIXES
set( CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES_SAV} )


mark_as_advanced(LZ4_LIBRARY LZ4_LIBRARY_DIRS LZ4_INCLUDE_DIRS LZ4_STATIC_LIB LZ4_SHARED_LIB)
