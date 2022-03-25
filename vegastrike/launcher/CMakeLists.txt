#set_property(DIRECTORY PROPERTY INCLUDE_DIRECTORIES "")
SET(VSLAUNCHER_COMMON_SOURCES
                    general.cpp
                    main.cpp
                    ../src/log.cpp
                    ../src/unicode.cpp
                    ../src/unicode_combine.cpp
                    ../src/common/common.cpp )

SET(VSLAUNCHER_GTK_SOURCES
                    saveinterface.cpp)

SET(VSLAUNCHER_NOGUI_SOURCES
                    nogui.cpp)

SET(VSLAUNCHER_LIBS)
SET(VSLAUNCHER_CPPFLAGS "-DVS_LOG_NO_XML")

if (WIN32 AND NOT VS_WIN32_CONSOLE)
    find_program(WINDRES windres)
    set(VSLAUNCHER_COMMON_SOURCES ${VSLAUNCHER_COMMON_SOURCES} launcher.rc)
    set(VSLAUNCHER_LIBS ${VSLAUNCHER_LIBS} "-mwindows")
endif()

if(MACOSX OR APPLE)
    find_library(APPLE_CARBON_LIBRARY Carbon)
    if (APPLE_CARBON_LIBRARY)
        message(STATUS "linking vslauncher with Carbon")
        set(VSLAUNCHER_LIBS ${VSLAUNCHER_LIBS} ${APPLE_CARBON_LIBRARY})
        set(VSLAUNCHER_CPPFLAGS ${VSLAUNCHER_CPPFLAGS} -DHAVE_CARBON)
    endif()
endif()

FIND_LIBRARY(DL_LIB dl)
if (DL_LIB)
    SET(VSLAUNCHER_LIBS ${VSLAUNCHER_LIBS} "${DL_LIB}")
endif()

#find GTK2
UNSET(GTK2_FIND_REQUIRED)
if (NOT VS_GTK_DISABLE)
    find_package(GTK2)
endif()
IF(GTK2_FOUND)
    SET(GTK_LIBS ${GTK2_LIBRARIES})
    SET(GTK_CFLAGS ${GTK2_DEFINITIONS} -DGTK)
    add_executable(vslauncher-gui ${VSLAUNCHER_COMMON_SOURCES} ${VSLAUNCHER_GTK_SOURCES})
    if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.18)
        set_source_files_properties(${VSLAUNCHER_GTK_SOURCES} PROPERTIES COMPILE_OPTIONS "${GTK_CFLAGS}")
    else()
        target_compile_options(vslauncher-gui PUBLIC "${GTK_CFLAGS}") # works also but set_s.._properties is more precise
    endif()
    target_link_libraries(vslauncher-gui ${GTK_LIBS})
    target_link_libraries(vslauncher-gui ${VSLAUNCHER_LIBS})
    target_compile_options(vslauncher-gui PUBLIC ${VSLAUNCHER_CPPFLAGS})
    add_dependencies(vslauncher-gui get_scm_revision)
ELSE(GTK2_FOUND)
    message("!! vslauncher: Cant get GTK2 found, vslauncher-gui will not be be built")
ENDIF(GTK2_FOUND)

add_executable(vslauncher ${VSLAUNCHER_COMMON_SOURCES} ${VSLAUNCHER_NOGUI_SOURCES})

target_link_libraries(vslauncher ${VSLAUNCHER_LIBS})
target_compile_options(vslauncher PUBLIC ${VSLAUNCHER_CPPFLAGS})

#add_definitions(${GTK_CFLAGS} ${DLG_CFLAGS})
include_directories(${vsUTCS_SOURCE_DIR}/launcher ${vsUTCS_SOURCE_DIR}/src/common ${vsUTCS_SOURCE_DIR} ${vsUTCS_BINARY_DIR} ${GTK2_INCLUDE_DIRS})

add_dependencies(vslauncher get_scm_revision)

