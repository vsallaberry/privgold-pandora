#set_property(DIRECTORY PROPERTY INCLUDE_DIRECTORIES "")
SET(VSSETUP_COMMON_SOURCES
                    src/c/setup.cpp
                    src/include/central.cpp
                    src/include/file.cpp
                    src/include/general.cpp
                    ../src/common/common.cpp
                    ../src/log.cpp )

SET(VSSETUP_GTK_SOURCES
                    src/include/display_gtk.cpp )

if (WIN32 AND NOT VS_WIN32_CONSOLE)
    set(VSSETUP_GTK_SOURCES ${VSSETUP_GTK_SOURCES} vega.rc)
endif()

SET(VSSETUP_DLG_SOURCES
                    src/include/display_dialog.cpp )


#find GTK2
UNSET(GTK2_FIND_REQUIRED)
if(NOT VS_GTK_DISABLE)
   find_package(GTK2)
endif()
IF(GTK2_FOUND)
    SET(GTK_LIBS ${GTK2_LIBRARIES})
    SET(GTK_CFLAGS ${GTK2_DEFINITIONS} -DGTK)
    add_executable(vssetup ${VSSETUP_COMMON_SOURCES} ${VSSETUP_GTK_SOURCES})
    if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.18)
        set_source_files_properties(${VSSETUP_GTK_SOURCES} PROPERTIES COMPILE_OPTIONS "${GTK_CFLAGS}")
    else()
        target_compile_options(vssetup PUBLIC "${GTK_CFLAGS}") # works also but set_s.._properties is more precise
    endif()
    target_link_libraries(vssetup ${GTK_LIBS})
    target_compile_options(vssetup PUBLIC "-DVS_LOG_NO_XML")
    if (WIN32 AND (CYGWIN OR MINGW) AND NOT VS_WIN32_CONSOLE)
        find_program(WINDRES windres)
        target_link_libraries(vssetup "-mwindows")
    endif()
    add_dependencies(vssetup get_scm_revision)
ELSE(GTK2_FOUND)
     message("!! vssetup: Cant get GTK2 found")
ENDIF(GTK2_FOUND)

#find LIBDIALOG
IF(NOT DEFINED DIALOG_FOUND OR DIALOG_FOUND)
  SET(DLG_CONFIG_NAMES "dialog-config")
  find_program(DLG_CONFIG_EXECUTABLE NAMES ${DLG_CONFIG_NAMES} DOC "dialog-config executable")
  mark_as_advanced(DLG_CONFIG_EXECUTABLE)
endif()
if (DLG_CONFIG_EXECUTABLE)
    execute_process(COMMAND ${DLG_CONFIG_EXECUTABLE} --cflags
        OUTPUT_VARIABLE DLG_CONFIG_EXEC_STRING OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_VARIABLE _DLG_CONFIG_EXEC_ERROR ERROR_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE _DLG_CONFIG_EXEC_RESULT
        )
    if (NOT _DLG_CONFIG_EXEC_RESULT EQUAL 0)
        #nothing
    else()
        SET(DLG_DEFINITIONS ${DLG_CONFIG_EXEC_STRING})
        unset(_DLG_CONFIG_EXEC_RESULT)
        execute_process(COMMAND ${DLG_CONFIG_EXECUTABLE} --libs
            OUTPUT_VARIABLE DLG_CONFIG_EXEC_STRING OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_VARIABLE _DLG_CONFIG_EXEC_ERROR ERROR_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE _DLG_CONFIG_EXEC_RESULT
        )
        if (NOT _DLG_CONFIG_EXEC_RESULT EQUAL 0)
            #nothing
        else()
            SET(DLG_LIBRARIES ${DLG_CONFIG_EXEC_STRING})
            SET(DIALOG_FOUND 1)
            message("-- vssetup: Found libdialog (${DLG_DEFINITIONS}|${DLG_LIBRARIES})")
        endif()
    endif ()
    unset(_DLG_CONFIG_EXEC_RESULT)
endif ()
IF(DIALOG_FOUND)
    SET(DLG_LIBS ${DLG_LIBRARIES})
    SET(DLG_CFLAGS ${DLG_DEFINITIONS} -DCONSOLE)
    add_executable(vssetup_dlg ${VSSETUP_COMMON_SOURCES} ${VSSETUP_DLG_SOURCES})
    if (FALSE AND CMAKE_VERSION VERSION_GREATER_EQUAL 3.18) ## CAN'T have the srcfile props setup.cpp for only vssetup_dlg
        set_source_files_properties(${VSSETUP_DLG_SOURCES} PROPERTIES COMPILE_OPTIONS "${DLG_CFLAGS}")
        set_source_files_properties(src/c/setup.cpp TARGET_DIRECTORY vssetup_dlg PROPERTIES COMPILE_OPTIONS "-DCONSOLE")
    else()
        target_compile_options(vssetup_dlg PUBLIC "${DLG_CFLAGS}") # works also but set_s.._properties is more precise
    endif()
    target_link_libraries(vssetup_dlg ${DLG_LIBS})
    target_compile_options(vssetup_dlg PUBLIC "-DVS_LOG_NO_XML")
    add_dependencies(vssetup_dlg get_scm_revision)
ELSE(DIALOG_FOUND)
    message("!! vssetup: Cant get libdialog found")
ENDIF(DIALOG_FOUND)

IF(NOT GTK2_FOUND AND NOT DIALOG_FOUND)
    message(FATAL_ERROR "!! no switable library found for vssetup")
ENDIF(NOT GTK2_FOUND AND NOT DIALOG_FOUND)

#add_definitions(${GTK_CFLAGS} ${DLG_CFLAGS})
include_directories(${vsUTCS_SOURCE_DIR}/setup/src/include ${vsUTCS_SOURCE_DIR}/src/common ${vsUTCS_BINARY_DIR} ${GTK2_INCLUDE_DIRS})

