
set(VSRMAKE_SOURCES
    common.cpp
    vsrmake.cpp)

set(VSREXTRACT_SOURCES
    common.cpp
    vsrextract.cpp)

set(UNICODE_CONV_SOURCES
    ../src/unicode.cpp
    unicode-conv.cpp)

add_executable(vsrextract ${VSREXTRACT_SOURCES})
add_executable(vsrmake ${VSRMAKE_SOURCES})

add_executable(unicode-conv ${UNICODE_CONV_SOURCES})
target_compile_options(unicode-conv PUBLIC "-DVS_LOG_NO_XML")
target_include_directories(unicode-conv PUBLIC .. ../src)
#target_link_libraries(test ${TST_LIB})

