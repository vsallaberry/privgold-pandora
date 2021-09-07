# - Try to find FFmpeg
# Once done this will define
#
#  FFMPEG_FOUND - system has FFmpeg
#  FFMPEG_INCLUDE_DIRS - the FFmpeg include directory
#  FFMPEG_LIBRARIES - Link these to use FFmpeg
#  FFMPEG_DEFINITIONS - Compiler switches required for using FFmpeg
#
#  Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

if(NOT FFMPEG_DEBUG_ENABLE)
    set(FFMPEG_DEBUG_ENABLE OFF)
endif(NOT FFMPEG_DEBUG_ENABLE)

macro(FFMPEG_DEBUG _message)
    if (FFMPEG_DEBUG_ENABLE)
        message(STATUS "(DEBUG) ${_message}")
    endif ()
endmacro(FFMPEG_DEBUG _message)



if (FFMPEG_LIBRARIES AND FFMPEG_INCLUDE_DIRS)
  # in cache already
  set(FFMPEG_FOUND TRUE)
else (FFMPEG_LIBRARIES AND FFMPEG_INCLUDE_DIRS)
	SET(FFMPEG_LIBRARIES)
    UNSET(FFMPEG_INCLUDE_DIR CACHE)
      ## ^ will repeat the find_path() each time. if commented, need the HAVE_LIBAV... cached.
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  include(FindPkgConfig)

  pkg_check_modules(ffmpeg1 REQUIRED libavcodec )
  SET(_FFMPEGIncDir ${ffmpeg1_INCLUDE_DIRS})
  SET(_FFMPEGLinkDir ${ffmpeg1_LIBRARY_DIRS})
  SET(_FFMPEGLinkFlags ${ffmpeg1_LDFLAGS})
  SET(_FFMPEGCflags ${ffmpeg1_CFLAGS})
  string(REGEX REPLACE "\(^|[ ]\)-I[^ ]*" "\\1" _FFMPEGCflags "${_FFMPEGCflags}")
  set(FFMPEG_DEFINITIONS ${_FFMPEGCflags})
  FFMPEG_DEBUG("libavcodec_INC: ${_FFMPEGIncDir}")
  FFMPEG_DEBUG("libavcodec_CFLAGS: ${_FFMPEGCflags}")
  FFMPEG_DEBUG("CMAKE_LIBRARY_PATH: ${CMAKE_LIBRARY_PATH}")
  FFMPEG_DEBUG("CMAKE_INCLUDE_PATH: ${CMAKE_INCLUDE_PATH}")
  FFMPEG_DEBUG("SYSTEM_ENVIRONMENT_PATH_LIB: ${SYSTEM_ENVIRONMENT_PATH_LIB}")
  FFMPEG_DEBUG("SYSTEM_ENVIRONMENT_PATH_PATH: ${SYSTEM_ENVIRONMENT_PATH_LIB}")
  FFMPEG_DEBUG("CMAKE_SYSTEM_LIBRARY_PATH: ${CMAKE_SYSTEM_LIBRARY_PATH}")
  #FFMPEG_DEBUG("
  #FFMPEG_DEBUG("

  string(REGEX REPLACE "([^ ])([ ]|$)" "\\1/include " _TMP_VS_FIND_PREFIX_MORE_PATHS "${VS_FIND_PREFIX_MORE_PATHS}")
  FFMPEG_DEBUG("VS_FIND more prefixes:: ${_TMP_VS_FIND_PREFIX_MORE_PATHS}")

  find_path(FFMPEG_INCLUDE_DIR
    NAMES
      ffmpeg/avcodec.h
    PATHS
      ${_FFMPEGIncDir}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
      ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
        NO_DEFAULT_PATH
    #PATH_SUFFIXES
      #ffmpeg
      #libavcodec
  )

  IF(FFMPEG_INCLUDE_DIR)
      message("-- ffmpeg ${FFMPEG_INCLUDE_DIR}/include/ffmpeg/<name>.h style")
  ELSE(FFMPEG_INCLUDE_DIR)
    find_path(FFMPEG_INCLUDE_DIR
      NAMES
        libavcodec/avcodec.h
      PATHS
        ${_FFMPEGIncDir}
        /usr/include
        /usr/local/include
        /opt/local/include
        /sw/include
        ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
      #PATH_SUFFIXES
        #ffmpeg
        #libavcodec
      NO_DEFAULT_PATH
    )
    IF(FFMPEG_INCLUDE_DIR)
        message("-- ffmpeg ${FFMPEG_INCLUDE_DIR}/include/libav<name>/<name>.h style")
        FFMPEG_DEBUG("_FFMPEGIncDir ${_FFMPEGIncDir}")
        set(HAVE_LIBAVCODEC_AVCODEC_H 1 CACHE STRING "ffmpeg avc lib../..h" FORCE)
        set(HAVE_LIBAVCODEC_AVUTIL_H 1 CACHE STRING "ffmpeg avu lib../..h" FORCE)
        set(HAVE_LIBAVFORMAT_AVFORMAT_H 1 CACHE STRING "ffmpeg avf lib../..h" FORCE )
        set(HAVE_LIBSWSCALE_SWSCALE_H 1 CACHE STRING "ffmpeg sws lib../..h" FORCE)
        set(HAVE_LIBAVFILTER_AVFILTER_H 1 CACHE STRING "ffmpeg avfl lib../..h" FORCE)
        set(HAVE_LIBAVFORMAT_AVIO_H 1 CACHE STRING "ffmpeg avf lib../..h" FORCE)
        set(HAVE_LIBAVDEVICE_AVDEVICE_H 1 CACHE STRING "ffmpeg avd lib../..h" FORCE)
        set(HAVE_LIBSWRESAMPLE_SWRESAMPLE_H 1 CACHE STRING "ffmpeg swr lib../..h" FORCE)
      ENDIF(FFMPEG_INCLUDE_DIR)
  ENDIF(FFMPEG_INCLUDE_DIR)

  string(REGEX REPLACE "([^ ])([ ]|$)" "\\1/lib " _TMP_VS_FIND_PREFIX_MORE_PATHS "${VS_FIND_PREFIX_MORE_PATHS}")

  find_library(AVUTIL_LIBRARY
    NAMES
      avutil
    PATHS
      ${_FFMPEGLinkDir}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
      ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
    NO_DEFAULT_PATH
  )
  FFMPEG_DEBUG("AVU_LIB ${AVUTIL_LIBRARY}")

  find_library(AVCODEC_LIBRARY
    NAMES
      avcodec
    PATHS
      ${_FFMPEGLinkDir}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
      ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
     NO_DEFAULT_PATH
  )
  FFMPEG_DEBUG("AVC_LIB ${AVCODEC_LIBRARY}")

  find_library(AVFORMAT_LIBRARY
    NAMES
      avformat
    PATHS
      ${_FFMPEGLinkDir}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
      ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
    NO_DEFAULT_PATH
  )
  FFMPEG_DEBUG("AVF_LIB ${AVFORMAT_LIBRARY}")

  find_library(SWSCALE_LIBRARY
    NAMES
      swscale
    PATHS
      ${_FFMPEGLinkDir}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
      ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
    NO_DEFAULT_PATH
  )
  FFMPEG_DEBUG("SWS_LIB ${SWSCALE_LIBRARY}")

  find_library(BZ2_LIBRARY
    NAMES
      bz2
    PATHS
      ${_FFMPEGLinkDir}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
      ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
    NO_DEFAULT_PATH
  )
  FFMPEG_DEBUG("BZ2_LIB ${BZ2_LIBRARY}")



  if (AVUTIL_LIBRARY)
    set(FFMPEG_LIBRARIES
      ${FFMPEG_LIBRARIES}
      ${AVUTIL_LIBRARY}
    )
  endif (AVUTIL_LIBRARY)

  if (AVCODEC_LIBRARY)
    set(FFMPEG_LIBRARIES
      ${FFMPEG_LIBRARIES}
      ${AVCODEC_LIBRARY}
    )
  endif (AVCODEC_LIBRARY)

  if (AVFORMAT_LIBRARY)
    set(FFMPEG_LIBRARIES
      ${FFMPEG_LIBRARIES}
      ${AVFORMAT_LIBRARY}
    )
  endif (AVFORMAT_LIBRARY)

  if (SWSCALE_LIBRARY)
    set(FFMPEG_LIBRARIES
      ${FFMPEG_LIBRARIES}
      ${SWSCALE_LIBRARY}
    )
  endif (SWSCALE_LIBRARY)

  if (BZ2_LIBRARY)
    set(FFMPEG_LIBRARIES
      ${FFMPEG_LIBRARIES}
      ${BZ2_LIBRARY}
    )
  endif (BZ2_LIBRARY)


  set(FFMPEG_INCLUDE_DIRS
    ${FFMPEG_INCLUDE_DIR}
  )

  if (FFMPEG_INCLUDE_DIRS AND FFMPEG_LIBRARIES)
     set(FFMPEG_FOUND TRUE)
  endif (FFMPEG_INCLUDE_DIRS AND FFMPEG_LIBRARIES)

  if (FFMPEG_FOUND)
    if (NOT FFMPEG_FIND_QUIETLY)
        message(STATUS "Found FFmpeg: ${FFMPEG_INCLUDE_DIRS} ${FFMPEG_LIBRARIES}")
    endif (NOT FFMPEG_FIND_QUIETLY)
  else (FFMPEG_FOUND)
    if (FFMPEG_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find FFmpeg")
    endif (FFMPEG_FIND_REQUIRED)
  endif (FFMPEG_FOUND)

  # show the FFMPEG_INCLUDE_DIRS and FFMPEG_LIBRARIES variables only in the advanced view
  mark_as_advanced(FFMPEG_INCLUDE_DIRS FFMPEG_LIBRARIES)

endif (FFMPEG_LIBRARIES AND FFMPEG_INCLUDE_DIRS)

