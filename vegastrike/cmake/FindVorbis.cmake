# - Try to find Vorbis and Ogg
# Once done this will define
#
#  Vorbis_FOUND - system has vorbis
#  Vorbis_INCLUDE_DIRS - the vorbis include directory
#  Vorbis_LIBRARIES - Link these to use vorbis




if (Vorbis_LIBRARIES AND Vorbis_INCLUDE_DIRS)
  # in cache already
  set(Vorbis_FOUND TRUE)
else (Vorbis_LIBRARIES AND Vorbis_INCLUDE_DIRS)
  SET(Vorbis_FOUND FALSE)

  string(REGEX REPLACE "([^ ])([ ]|$)" "\\1/include " _TMP_VS_FIND_PREFIX_MORE_PATHS "${VS_FIND_PREFIX_MORE_PATHS}")

  find_path(vorbis_INCLUDE_DIR
    NAMES
      vorbisfile.h
    PATHS
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
      ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
    PATH_SUFFIXES
      vorbis
  )

  find_path(ogg_INCLUDE_DIR
  	NAMES
  		ogg.h
  	PATHS
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
      ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
    PATH_SUFFIXES
      ogg
  )

  string(REGEX REPLACE "([^ ])([ ]|$)" "\\1/lib " _TMP_VS_FIND_PREFIX_MORE_PATHS "${VS_FIND_PREFIX_MORE_PATHS}")

  find_library(vorbis_LIBRARY
    NAMES
      vorbis
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
      ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
  )

 find_library(vorbisfile_LIBRARY
    NAMES
      vorbisfile
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
      ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
  )

 find_library(ogg_LIBRARY
    NAMES
      ogg
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
      ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
  )

  SET(Vorbis_INCLUDE_DIRS ${vorbis_INCLUDE_DIR} ${ogg_INCLUDE_DIR})
  SET(Vorbis_LIBRARIES ${vorbis_LIBRARY} ${vorbisfile_LIBRARY} ${ogg_LIBRARY})

  IF(vorbis_LIBRARY AND ogg_LIBRARY AND vorbisfile_LIBRARY)
  	set(Vorbis_FOUND TRUE)
  ENDIF(vorbis_LIBRARY AND ogg_LIBRARY AND vorbisfile_LIBRARY)

  if (Vorbis_FOUND)
    if (NOT Vorbis_FIND_QUIETLY)
      message(STATUS "Found Vorbis: ${Vorbis_LIBRARIES}")
    endif (NOT Vorbis_FIND_QUIETLY)
  else (Vorbis_FOUND)
    if (Vorbis_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find Vorbis")
    endif (Vorbis_FIND_REQUIRED)
  endif (Vorbis_FOUND)

  # show the Vorbis_INCLUDE_DIRS and Vorbis_LIBRARIES variables only in the advanced view
  mark_as_advanced(Vorbis_INCLUDE_DIRS Vorbis_LIBRARIES)

endif (Vorbis_LIBRARIES AND Vorbis_INCLUDE_DIRS)

