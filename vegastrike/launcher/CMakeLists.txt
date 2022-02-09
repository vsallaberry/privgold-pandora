#set_property(DIRECTORY PROPERTY INCLUDE_DIRECTORIES "")
SET(VSLAUNCHER_COMMON_SOURCES
                    general.cpp
                    main.cpp
                    ../src/log.cpp
                    ../src/common/common.cpp )

SET(VSLAUNCHER_GTK_SOURCES
                    saveinterface.cpp)

SET(VSLAUNCHER_NOGUI_SOURCES
                    nogui.cpp)

#find GTK2
UNSET(GTK2_FIND_REQUIRED)
if (NOT VS_GTK_DISABLE)
    find_package(GTK2)
endif()
IF(GTK2_FOUND)
    SET(GTK_LIBS ${GTK2_LIBRARIES})
    SET(GTK_CFLAGS ${GTK2_DEFINITIONS} -DGTK)
    add_executable(vslauncher ${VSLAUNCHER_COMMON_SOURCES} ${VSLAUNCHER_GTK_SOURCES})
    if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.18)
        set_source_files_properties(${VSLAUNCHER_GTK_SOURCES} PROPERTIES COMPILE_OPTIONS "${GTK_CFLAGS}")
    else()
        target_compile_options(vslauncher PUBLIC "${GTK_CFLAGS}") # works also but set_s.._properties is more precise
    endif()
    target_link_libraries(vslauncher ${GTK_LIBS})
ELSE(GTK2_FOUND)
    message("!! vslauncher: Cant get GTK2 found, vslauncher will be built without GUI")
    add_executable(vslauncher ${VSLAUNCHER_COMMON_SOURCES} ${VSLAUNCHER_NOGUI_SOURCES})
ENDIF(GTK2_FOUND)

if (WIN32 AND (CYGWIN OR MINGW) AND NOT VS_WIN32_CONSOLE)
    target_link_libraries(vslauncher "-mwindows")
endif()

target_compile_options(vslauncher PUBLIC "-DVS_LOG_NO_XML")

#add_definitions(${GTK_CFLAGS} ${DLG_CFLAGS})
include_directories(${vsUTCS_SOURCE_DIR}/launcher ${vsUTCS_SOURCE_DIR}/src/common ${vsUTCS_SOURCE_DIR} ${vsUTCS_BINARY_DIR} ${GTK2_INCLUDE_DIRS})

