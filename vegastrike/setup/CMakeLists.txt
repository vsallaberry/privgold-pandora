#set_property(DIRECTORY PROPERTY INCLUDE_DIRECTORIES "")
SET(VSSETUP_COMMON_SOURCES
                    src/c/setup.cpp
                    src/include/central.cpp
                    src/include/file.cpp
                    src/include/general.cpp
                    ../src/common/common.cpp )

SET(VSSETUP_GTK_SOURCES
                    src/include/display_gtk.cpp )

SET(VSSETUP_DLG_SOURCES
                    src/include/display_dialog.cpp )


#find GTK2
UNSET(GTK2_FIND_REQUIRED)
if(NOT DEFINED GTK_FOUND OR GTK_FOUND)
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
ELSE(GTK2_FOUND)
     message("!! Cant get GTK2 found")
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
            message("-- Found libdialog (${DLG_DEFINITIONS}|${DLG_LIBRARIES})")
        endif()
    endif ()
    unset(_DLG_CONFIG_EXEC_RESULT)
endif ()
IF(DIALOG_FOUND)
    SET(DLG_LIBS ${DLG_LIBRARIES})
    SET(DLG_CFLAGS ${DLG_DEFINITIONS} -DCONSOLE)
    add_executable(vssetup_dlg ${VSSETUP_COMMON_SOURCES} ${VSSETUP_DLG_SOURCES})
    if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.18)
        set_source_files_properties(${VSSETUP_DLG_SOURCES} PROPERTIES COMPILE_OPTIONS "${DLG_CFLAGS}")
    else()
        target_compile_options(vssetup_dlg PUBLIC "${DLG_CFLAGS}") # works also but set_s.._properties is more precise
    endif()
    target_link_libraries(vssetup_dlg ${DLG_LIBS})
ELSE(DIALOG_FOUND)
    message("!! Cant get libdialog found")
ENDIF(DIALOG_FOUND)

IF(NOT GTK2_FOUND AND NOT DIALOG_FOUND)
    message(FATAL_ERROR "!! no switable library found for vssetup")
ENDIF(NOT GTK2_FOUND AND NOT DIALOG_FOUND)

#add_definitions(${GTK_CFLAGS} ${DLG_CFLAGS})
include_directories(${vsUTCS_SOURCE_DIR}/setup/src/include ${vsUTCS_SOURCE_DIR}/src/common ${vsUTCS_BINARY_DIR} ${GTK2_INCLUDE_DIRS})

