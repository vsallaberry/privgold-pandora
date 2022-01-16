# - Try to find GTK2
# Once done this will define
#
#  GTK2_FOUND - System has Boost
#  GTK2_INCLUDE_DIRS - GTK2 include directory
#  GTK2_LIBRARIES - Link these to use GTK2
#  GTK2_LIBRARY_DIRS - The path to where the GTK2 library files are.
#  GTK2_DEFINITIONS - Compiler switches required for using GTK2
#
#  Copyright (c) 2007 Andreas Schneider <mail@cynapses.org>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

if (NOT DEFINED GTK2_CMAKE_DEBUG)
    set(GTK2_CMAKE_DEBUG OFF)
endif()

macro(GTK2_DEBUG_MESSAGE _message)
    if (GTK2_CMAKE_DEBUG)
        message(STATUS "(DEBUG) ${_message}")
    endif (GTK2_CMAKE_DEBUG)
endmacro(GTK2_DEBUG_MESSAGE _message)

if (GTK2_LIBRARIES AND GTK2_INCLUDE_DIRS)
  # in cache already
  set(GTK2_FOUND TRUE)
else (GTK2_LIBRARIES AND GTK2_INCLUDE_DIRS)
  if (UNIX OR CYGWIN OR MINGW)
    # use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls
    #include(FindPkgConfig)
    find_package(PkgConfig)
    if (GTK2_FIND_REQUIRED)
        set(GTK2_PKG_REQUIRED REQUIRED)
        set(GTK2_PKG_ERROR SEND_ERROR)
    else(GTK2_FIND_REQUIRED)
        set(GTK2_PKG_REQUIRED "")
        set(GTK2_PKG_ERROR "")
    endif(GTK2_FIND_REQUIRED)

    pkg_check_modules(gtk2.0 ${GTK2_PKG_REQUIRED} gtk+-2.0)
    GTK2_DEBUG_MESSAGE("pkg: gtks_inc = ${gtk2.0_INCLUDE_DIRS}")
    GTK2_DEBUG_MESSAGE("pkg: gtks_libs = ${gtk2.0_LIBRARY_DIRS}")
	SET(_GTK2IncDir ${gtk2.0_INCLUDE_DIRS})
	SET(_GTK2LinkDir ${gtk2.0_LIBRARY_DIRS})
	SET(_GTK2LinkFlags ${gtk2.0_LDFLAGS})
	SET(_GTK2Cflags ${gtk2.0_CFLAGS})

    # replace vim pattern : %s/\(VS_FIND[^}]*}\).*/_TMP_\1/gc
    string(REGEX REPLACE "([^ ])([ ]|$)" "\\1/include/gtk-2.0 " _TMP_VS_FIND_PREFIX_MORE_PATHS "${VS_FIND_PREFIX_MORE_PATHS}")

    find_path(GTK2_GTK_INCLUDE_DIR
      NAMES
        gtk/gtk.h
      HINTS
        ${_GTK2IncDir}
        ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
      PATHS
        $ENV{GTK2_HOME}
        /usr/include/gtk-2.0
        /usr/local/include/gtk-2.0
        /opt/include/gtk-2.0
        /opt/gnome/include/gtk-2.0
        /sw/include/gtk-2.0
        /opt/local/include/gtk-2.0
    )
    gtk2_debug_message("GTK2_GTK_INCLUDE_DIR is ${GTK2_GTK_INCLUDE_DIR}")

    # Some Linux distributions (e.g. Red Hat) have glibconfig.h
    # and glib.h in different directories, so we need to look
    # for both.
    #  - Atanas Georgiev <atanas@cs.columbia.edu>
    pkg_check_modules(glib2.0 ${GTK2_PKG_REQUIRED} glib-2.0)
    SET(_GLIB2IncDir ${glib2.0_INCLUDE_DIRS})
    SET(_GLIB2LinkDir ${glib2.0_LIBRARY_DIRS})
    SET(_GLIB2LinkFlags ${glib2.0_LDFLAGS})
    SET(_GLIB2Cflags ${glib2.0_CFLAGS})

    pkg_check_modules(gmod2.0 ${GTK2_PKG_REQUIRED} gmodule-2.0)
    SET(_GMODULE2IncDir ${gmod2.0_INCLUDE_DIRS})
    SET(_GMODULE2LinkDir ${gmod2.0_LIBRARY_DIRS})
    SET(_GMODULE2LinkFlags ${gmod2.0_LDFLAGS})
    SET(_GMODULE2Cflags ${gmod2.0_CFLAGS})

    string(REGEX REPLACE "([^ ])([ ]|$)" "\\1/include/glib-2.0 " _TMP_VS_FIND_PREFIX_MORE_PATHS "${VS_FIND_PREFIX_MORE_PATHS}")

    find_path(GTK2_GLIBCONFIG_INCLUDE_DIR
      NAMES
        glibconfig.h
      HINTS
        ${_GLIB2IncDir}
        ${_GMODULE2IncDir}
        ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
        ${_TMP_VS_FIND_PREFIX_MORE_PATHS}/../../lib/glib-2.0
      PATHS
	$ENV{GTK2_HOME}/../glib-2.0
	$ENV{GTK2_HOME}/../../lib/glib-2.0/include
        /opt/gnome/lib64/glib-2.0/include
        /opt/gnome/lib/glib-2.0/include
        /opt/lib/glib-2.0/include
        /usr/lib64/glib-2.0/include
        /usr/lib/glib-2.0/include
        /sw/lib/glib-2.0/include
        /opt/local/include/glib-2.0
    )
    gtk2_debug_message("GTK2_GLIBCONFIG_INCLUDE_DIR is ${GTK2_GLIBCONFIG_INCLUDE_DIR}")

    string(REGEX REPLACE "([^ ])([ ]|$)" "\\1/include/glib-2.0 " _TMP_VS_FIND_PREFIX_MORE_PATHS "${VS_FIND_PREFIX_MORE_PATHS}")

    find_path(GTK2_GLIB_INCLUDE_DIR
      NAMES
        glib.h
      HINTS
        ${_GLIB2IncDir}
        ${_GMODULE2IncDir}
        ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
      PATHS
	$ENV{GTK2_HOME}/../glib-2.0
        /opt/include/glib-2.0
        /opt/gnome/include/glib-2.0
        /usr/include/glib-2.0
        /sw/include/glib-2.0
        /opt/local/include/glib-2.0
    )
    gtk2_debug_message("GTK2_GLIB_INCLUDE_DIR is ${GTK2_GLIB_INCLUDE_DIR}")

	pkg_check_modules(gdk2.0 ${GTK2_PKG_REQUIRED} gdk-2.0)
	SET(_GDK2IncDir ${gdk2.0_INCLUDE_DIRS})
	SET(_GDK2LinkDir ${gdk2.0_LIBRARY_DIRS})
	SET(_GDK2LinkFlags ${gdk2.0_LDFLAGS})
	SET(_GDK2Cflags ${gdk2.0_CFLAGS})

    string(REGEX REPLACE "([^ ])([ ]|$)" "\\1/include/gtk-2.0 " _TMP_VS_FIND_PREFIX_MORE_PATHS "${VS_FIND_PREFIX_MORE_PATHS}")

    find_path(GTK2_GDK_INCLUDE_DIR
      NAMES
        gdkconfig.h
      HINTS
        ${_GDK2IncDir}
        ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
      PATHS
	$ENV{GTK2_HOME}
	$ENV{GTK2_HOME}/gdk
	$ENV{GTK2_HOME}/../../lib/gtk-2.0/include
        /opt/gnome/lib/gtk-2.0/include
        /opt/gnome/lib64/gtk-2.0/include
        /opt/lib/gtk-2.0/include
        /usr/lib/gtk-2.0/include
        /usr/lib64/gtk-2.0/include
        /sw/lib/gtk-2.0/include
        /opt/local/include/gtk-2.0
    )
    gtk2_debug_message("GTK2_GDK_INCLUDE_DIR is ${GTK2_GDK_INCLUDE_DIR}")

    string(REGEX REPLACE "([^ ])([ ]|$)" "\\1/include/gdk-pixbuf-2.0 " _TMP_VS_FIND_PREFIX_MORE_PATHS "${VS_FIND_PREFIX_MORE_PATHS}")

    find_path(GTK2_GDKPIXBUF_INCLUDE_DIR
      NAMES
        gdk-pixbuf/gdk-pixbuf.h
      HINTS
        ${_GDK2IncDir}
        ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
      PATHS
	$ENV{GTK2_HOME}/../gdk-pixbuf-2.0
        /opt/gnome/lib/gdk-pixbuf-2.0/include
        /opt/gnome/lib64/gdk-pixbuf-2.0/include
        /opt/lib/gdk-pixbuf-2.0/include
        /usr/lib/gdk-pixbuf-2.0/include
        /usr/lib64/gdk-pixbuf-2.0/include
        /sw/lib/gdk-pixbuf-2.0/include
        /opt/local/include/gdk-pixbuf-2.0
    )
    gtk2_debug_message("GTK2_GDKPIXBUF_INCLUDE_DIR is ${GTK2_GDKPIXBUF_INCLUDE_DIR}")


    string(REGEX REPLACE "([^ ])([ ]|$)" "\\1/include " _TMP_VS_FIND_PREFIX_MORE_PATHS "${VS_FIND_PREFIX_MORE_PATHS}")

    find_path(GTK2_GTKGL_INCLUDE_DIR
      NAMES
        gtkgl/gtkglarea.h
      HINTS
        ${_GLIB2IncDir}
        ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
      PATHS
        /usr/include
        /usr/local/include
        /usr/openwin/share/include
        /opt/gnome/include
        /opt/include
        /sw/include
        /opt/local/include
    )
    gtk2_debug_message("GTK2_GTKGL_INCLUDE_DIR is ${GTK2_GTKGL_INCLUDE_DIR}")

    pkg_check_modules(Pango ${GTK2_PKG_REQUIRED} pango)
    SET(_PANGOIncDir ${Pango_INCLUDE_DIRS})
    SET(_PANGOLinkDir ${Pango_LIBRARY_DIRS})
    SET(_PANGOLinkFlags ${Pango_LDFLAGS})
    SET(_PANGOCflags ${Pango_CFLAGS})

    string(REGEX REPLACE "([^ ])([ ]|$)" "\\1/include/pango-1.0 " _TMP_VS_FIND_PREFIX_MORE_PATHS "${VS_FIND_PREFIX_MORE_PATHS}")

    find_path(GTK2_PANGO_INCLUDE_DIR
      NAMES
        pango/pango.h
      HINTS
        ${_PANGOIncDir}
        ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
      PATHS
	$ENV{GTK2_HOME}/../pango-1.0
        /usr/include/pango-1.0
        /opt/gnome/include/pango-1.0
        /opt/include/pango-1.0
        /sw/include/pango-1.0
        /opt/local/include/pango-1.0
    )
    gtk2_debug_message("GTK2_PANGO_INCLUDE_DIR is ${GTK2_PANGO_INCLUDE_DIR}")

    pkg_check_modules(Harfbuzz ${GTK2_PKG_REQUIRED} harfbuzz)
    SET(_HARFBUZZIncDir ${Harfbuzz_INCLUDE_DIRS})
    SET(_HARFBUZZLinkDir ${Harfbuzz_LIBRARY_DIRS})
    SET(_HARFBUZZLinkFlags ${Harfbuzz_LDFLAGS})
    SET(_HARFBUZZCflags ${Harfbuzz_CFLAGS})

    string(REGEX REPLACE "([^ ])([ ]|$)" "\\1/include/harfbuzz " _TMP_VS_FIND_PREFIX_MORE_PATHS "${VS_FIND_PREFIX_MORE_PATHS}")

    find_path(GTK2_HARFBUZZ_INCLUDE_DIR
      NAMES
        hb.h
      HINTS
        ${_HARFBUZZIncDir}
        ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
      PATHS
	$ENV{GTK2_HOME}/../harfbuzz
        /usr/include/harfbuzz
        /opt/gnome/include/harfbuzz
        /opt/include/harfbuzz
        /sw/include/harfbuzz
        /opt/local/include/harfbuzz
    )
    gtk2_debug_message("GTK2_HARFBUZZ_INCLUDE_DIR is ${GTK2_HARFBUZZ_INCLUDE_DIR}")

    pkg_check_modules(Cairo ${GTK2_PKG_REQUIRED} cairo)
    SET(_CAIROIncDir ${Cairo_INCLUDE_DIRS})
    SET(_CAIROLinkDir ${Cairo_LIBRARY_DIRS})
    SET(_CAIROLinkFlags ${Cairo_LDFLAGS})
    SET(_CAIROCflags ${Cairo_CFLAGS})

    string(REGEX REPLACE "([^ ])([ ]|$)" "\\1/include/cairo " _TMP_VS_FIND_PREFIX_MORE_PATHS "${VS_FIND_PREFIX_MORE_PATHS}")

    find_path(GTK2_CAIRO_INCLUDE_DIR
      NAMES
        cairo.h
      HINTS
        ${_CAIROIncDir}
        ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
      PATHS
	$ENV{GTK2_HOME}/../cairo
        /opt/gnome/include/cairo
        /usr/include
        /usr/include/cairo
        /opt/include
        /opt/include/cairo
        /sw/include
        /sw/include/cairo
        /opt/local/include/cairo
    )
    gtk2_debug_message("GTK2_CAIRO_INCLUDE_DIR is ${GTK2_CAIRO_INCLUDE_DIR}")

    pkg_check_modules(Atk ${GTK2_PKG_REQUIRED} atk)
    SET(_ATKIncDir ${Atk_INCLUDE_DIRS})
    SET(_ATKLinkDir ${Atk_LIBRARY_DIRS})
    SET(_ATKLinkFlags ${Atk_LDFLAGS})
    SET(_ATKCflags ${Atk_CFLAGS})

    string(REGEX REPLACE "([^ ])([ ]|$)" "\\1/include/atk-1.0 " _TMP_VS_FIND_PREFIX_MORE_PATHS "${VS_FIND_PREFIX_MORE_PATHS}")

    find_path(GTK2_ATK_INCLUDE_DIR
      NAMES
        atk/atk.h
      HINTS
        ${_ATKIncDir}
        ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
      PATHS
	$ENV{GTK2_HOME}/../atk-1.0
        /opt/gnome/include/atk-1.0
        /usr/include/atk-1.0
        /opt/include/atk-1.0
        /sw/include/atk-1.0
        /opt/local/include/atk-1.0
    )
    gtk2_debug_message("GTK2_ATK_INCLUDE_DIR is ${GTK2_ATK_INCLUDE_DIR}")

    string(REGEX REPLACE "([^ ])([ ]|$)" "\\1/lib " _TMP_VS_FIND_PREFIX_MORE_PATHS "${VS_FIND_PREFIX_MORE_PATHS}")

    find_library(GTK2_GTK_LIBRARY
      NAMES
        gtk-x11-2.0
        gtk-quartz-2.0
        gtk-win32-2.0
      HINTS
        ${_GTK2LinkDir}
        ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
      PATHS
        /usr/lib
        /usr/local/lib
        /usr/openwin/lib
        /usr/X11R6/lib
        /opt/gnome/lib
        /opt/lib
        /sw/lib
        /opt/local/lib
    )
    gtk2_debug_message("GTK2_GTK_LIBRARY is ${GTK2_GTK_LIBRARY}")

    find_library(GTK2_GDK_LIBRARY
      NAMES
        gdk-x11-2.0
        gdk-quartz-2.0
        gdk-win32-2.0
      HINTS
        ${_GDK2LinkDir}
        ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
      PATHS
        /usr/lib
        /usr/local/lib
        /usr/openwin/lib
        /usr/X11R6/lib
        /opt/gnome/lib
        /opt/lib
        /sw/lib
        /opt/local/lib
    )
    gtk2_debug_message("GTK2_GDK_LIBRARY is ${GTK2_GDK_LIBRARY}")

    find_library(GTK2_GDK_PIXBUF_LIBRARY
      NAMES
        gdk_pixbuf-2.0
      HINTS
        ${_GDK2LinkDir}
        ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
      PATHS
        /usr/lib
        /usr/local/lib
        /usr/openwin/lib
        /usr/X11R6/lib
        /opt/gnome/lib
        /opt/lib
        /sw/lib
        /opt/local/lib
    )
    gtk2_debug_message("GTK2_GDK_PIXBUF_LIBRARY is ${GTK2_GDK_PIXBUF_LIBRARY}")

    find_library(GTK2_GMODULE_LIBRARY
      NAMES
        gmodule-2.0
      HINTS
        ${_GMODULE2LinkDir}
        ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
      PATHS
        /usr/lib
        /usr/local/lib
        /usr/openwin/lib
        /usr/X11R6/lib
        /opt/gnome/lib
        /opt/lib
        /sw/lib
        /opt/local/lib
    )
    gtk2_debug_message("GTK2_GMODULE_LIBRARY is ${GTK2_GMODULE_LIBRARY}")

    find_library(GTK2_GTHREAD_LIBRARY
      NAMES
        gthread-2.0
      HINTS
        ${_GTK2LinkDir}
        ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
      PATHS
        /usr/lib
        /usr/local/lib
        /usr/openwin/lib
        /usr/X11R6/lib
        /opt/gnome/lib
        /opt/lib
        /sw/lib
        /opt/local/lib
    )
    gtk2_debug_message("GTK2_GTHREAD_LIBRARY is ${GTK2_GTHREAD_LIBRARY}")

    find_library(GTK2_GOBJECT_LIBRARY
      NAMES
        gobject-2.0
      HINTS
        ${_GTK2LinkDir}
        ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
      PATHS
        /usr/lib
        /usr/local/lib
        /usr/openwin/lib
        /usr/X11R6/lib
        /opt/gnome/lib
        /opt/lib
        /sw/lib
        /opt/local/lib
    )
    gtk2_debug_message("GTK2_GOBJECT_LIBRARY is ${GTK2_GOBJECT_LIBRARY}")

    find_library(GTK2_GLIB_LIBRARY
      NAMES
        glib-2.0
      HINTS
        ${_GLIB2LinkDir}
        ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
      PATHS
        /usr/lib
        /usr/local/lib
        /usr/openwin/lib
        /usr/X11R6/lib
        /opt/gnome/lib
        /opt/lib
        /sw/lib
        /opt/local/lib
    )
    gtk2_debug_message("GTK2_GLIB_LIBRARY is ${GTK2_GLIB_LIBRARY}")

    find_library(GTK2_GTKGL_LIBRARY
      NAMES
        gtkgl
      HINTS
        ${_GTK2LinkDir}
        ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
      PATHS
        /usr/lib
        /usr/local/lib
        /usr/openwin/lib
        /usr/X11R6/lib
        /opt/gnome/lib
        /opt/lib
        /sw/lib
        /opt/local/lib
    )
    gtk2_debug_message("GTK2_GTKGL_LIBRARY is ${GTK2_GTKGL_LIBRARY}")


    find_library(GTK2_PANGO_LIBRARY
      NAMES
        pango-1.0
      HINTS
        ${_PANGOLinkDir}
        ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
      PATHS
        /usr/lib
        /usr/local/lib
        /usr/openwin/lib
        /usr/X11R6/lib
        /opt/gnome/lib
        /opt/lib
        /sw/lib
        /opt/local/lib
    )
    gtk2_debug_message("GTK2_PANGO_LIBRARY is ${GTK2_PANGO_LIBRARY}")

    find_library(GTK2_CAIRO_LIBRARY
      NAMES
        pangocairo-1.0
      HINTS
        ${_CAIROLinkDir}
        ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
      PATHS
        /usr/lib
        /usr/local/lib
        /usr/openwin/lib
        /usr/X11R6/lib
        /opt/gnome/lib
        /opt/lib
        /sw/lib
        /opt/local/lib
    )
    gtk2_debug_message("GTK2_PANGO_LIBRARY is ${GTK2_CAIRO_LIBRARY}")

    find_library(GTK2_ATK_LIBRARY
      NAMES
        atk-1.0
      HINTS
        ${_ATKLinkDir}
        ${_TMP_VS_FIND_PREFIX_MORE_PATHS}
      PATHS
        /usr/lib
        /usr/local/lib
        /usr/openwin/lib
        /usr/X11R6/lib
        /opt/gnome/lib
        /opt/lib
        /sw/lib
        /opt/local/lib
    )
    gtk2_debug_message("GTK2_ATK_LIBRARY is ${GTK2_ATK_LIBRARY}")

    if (GTK2_GTK_LIBRARY AND GTK2_GTK_INCLUDE_DIR)
        if (GTK2_GDK_LIBRARY AND GTK2_GDK_PIXBUF_LIBRARY AND GTK2_GDK_INCLUDE_DIR) # AND GTK2_GDKPIXBUF_INCLUDE_DIR)
        if (GTK2_GMODULE_LIBRARY)
          if (GTK2_GTHREAD_LIBRARY)
            if (GTK2_GOBJECT_LIBRARY)
                if (GTK2_PANGO_LIBRARY AND GTK2_PANGO_INCLUDE_DIR)
                  if (GTK2_CAIRO_LIBRARY AND GTK2_CAIRO_INCLUDE_DIR)
                    if (GTK2_ATK_LIBRARY AND GTK2_ATK_INCLUDE_DIR)
                      # set GTK2 includes
                      if (NOT GTK2_HARFBUZZ_INCLUDE_DIR)
                          set(GTK2_HARFBUZZ_INCLUDE_DIR "")
                      endif()
                      set(GTK2_INCLUDE_DIRS
                        ${GTK2_GTK_INCLUDE_DIR}
                        ${GTK2_GLIBCONFIG_INCLUDE_DIR}
                        ${GTK2_GLIB_INCLUDE_DIR}
                        ${GTK2_GDK_INCLUDE_DIR}
                        ${GTK2_GDKPIXBUF_INCLUDE_DIR}
                        ${GTK2_PANGO_INCLUDE_DIR}
                        ${GTK2_CAIRO_INCLUDE_DIR}
                        ${GTK2_ATK_INCLUDE_DIR}
                        ${GTK2_HARFBUZZ_INCLUDE_DIR}
                      )

                      # set GTK2 libraries
                      set (GTK2_LIBRARIES
                        ${GTK2_GTK_LIBRARY}
                        ${GTK2_GDK_LIBRARY}
                        ${GTK2_GDK_PIXBUF_LIBRARY}
                        ${GTK2_GMODULE_LIBRARY}
                        ${GTK2_GTHREAD_LIBRARY}
                        ${GTK2_GOBJECT_LIBRARY}
                        ${GTK2_PANGO_LIBRARY}
                        ${GTK2_CAIRO_LIBRARY}
                        ${GTK2_ATK_LIBRARY}
                      )

                      # check for gtkgl support
                      if (GTK2_GTKGL_LIBRARY AND GTK2_GTKGL_INCLUDE_DIR)
                        set(GTK2_GTKGL_FOUND TRUE)

                        set(GTK2_INCLUDE_DIRS
                          ${GTK2_INCLUDE_DIR}
                          ${GTK2_GTKGL_INCLUDE_DIR}
                        )

                        set(GTK2_LIBRARIES
                          ${GTK2_LIBRARIES}
                          ${GTK2_GTKGL_LIBRARY}
                        )
                      endif (GTK2_GTKGL_LIBRARY AND GTK2_GTKGL_INCLUDE_DIR)

                    else (GTK2_ATK_LIBRARY AND GTK2_ATK_INCLUDE_DIR)
                      message(${GTK2_PKG_ERROR} "Could not find ATK")
                    endif (GTK2_ATK_LIBRARY AND GTK2_ATK_INCLUDE_DIR)
                  else (GTK2_CAIRO_LIBRARY AND GTK2_CAIRO_INCLUDE_DIR)
                    message(${GTK2_PKG_ERROR} "Could not find CAIRO")
                  endif (GTK2_CAIRO_LIBRARY AND GTK2_CAIRO_INCLUDE_DIR)
                else (GTK2_PANGO_LIBRARY AND GTK2_PANGO_INCLUDE_DIR)
                  message(${GTK2_PKG_ERROR} "Could not find PANGO")
                endif (GTK2_PANGO_LIBRARY AND GTK2_PANGO_INCLUDE_DIR)
            else (GTK2_GOBJECT_LIBRARY)
              message(${GTK2_PKG_ERROR} "Could not find GOBJECT")
            endif (GTK2_GOBJECT_LIBRARY)
          else (GTK2_GTHREAD_LIBRARY)
            message(${GTK2_PKG_ERROR} "Could not find GTHREAD")
          endif (GTK2_GTHREAD_LIBRARY)
        else (GTK2_GMODULE_LIBRARY)
          message(${GTK2_PKG_ERROR} "Could not find GMODULE")
        endif (GTK2_GMODULE_LIBRARY)
      else (GTK2_GDK_LIBRARY AND GTK2_GDK_PIXBUF_LIBRARY AND GTK2_GDK_INCLUDE_DIR)
        message(${GTK2_PKG_ERROR} "Could not find GDK (GDK_PIXBUF)")
      endif (GTK2_GDK_LIBRARY AND GTK2_GDK_PIXBUF_LIBRARY AND GTK2_GDK_INCLUDE_DIR)
    else (GTK2_GTK_LIBRARY AND GTK2_GTK_INCLUDE_DIR)
      message(${GTK2_PKG_ERROR} "Could not find GTK2")
    endif (GTK2_GTK_LIBRARY AND GTK2_GTK_INCLUDE_DIR)


    if (GTK2_INCLUDE_DIRS AND GTK2_LIBRARIES)
       set(GTK2_FOUND TRUE)
    endif (GTK2_INCLUDE_DIRS AND GTK2_LIBRARIES)

    if (GTK2_FOUND)
      if (NOT GTK2_FIND_QUIETLY)
        message(STATUS "Found GTK2: ${GTK2_LIBRARIES}")
      endif (NOT GTK2_FIND_QUIETLY)
    else (GTK2_FOUND)
      if (GTK2_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find GTK2")
      endif (GTK2_FIND_REQUIRED)
    endif (GTK2_FOUND)

    # show the GTK2_INCLUDE_DIRS and GTK2_LIBRARIES variables only in the advanced view
    mark_as_advanced(GTK2_INCLUDE_DIRS GTK2_LIBRARIES)

  endif (UNIX OR CYGWIN OR MINGW)
endif (GTK2_LIBRARIES AND GTK2_INCLUDE_DIRS)

