
set(TEST_SOURCES
    ../src/unicode.cpp
    ../src/multimap.cpp
    ../src/log.cpp
    ../src/xml_support.cpp
    test.cpp)

find_package(EXPAT REQUIRED)
IF(EXPAT_FOUND)
    SET(TOOLS_EXPAT_INCLUDE_DIRS ${EXPAT_INCLUDE_DIRS})
    SET(TOOLS_EXPAT_LIBRARIES ${EXPAT_LIBRARIES})
ELSE(EXPAT_FOUND)
	message("We can't find expat for XML stuff")
ENDIF(EXPAT_FOUND)

add_executable(test ${TEST_SOURCES})
#if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.18)
    #set_source_files_properties(${VSSETUP_GTK_SOURCES} PROPERTIES COMPILE_OPTIONS "${GTK_CFLAGS}")
#else()
    #target_compile_options(vssetup_dlg PUBLIC "${DLG_CFLAGS}") # works also but set_s.._properties is more precise
#endif()

target_compile_options(test PUBLIC "-DVS_LOG_NO_XML" "-DVS_UNIT_TESTS" "-DVS_UTF8_ITERATOR_TESTS")
target_include_directories(test PUBLIC .. ../src ${TOOLS_EXPAT_INCLUDE_DIRS})
target_link_libraries(test ${TST_LIB})


