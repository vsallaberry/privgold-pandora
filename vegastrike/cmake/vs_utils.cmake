# VSA 2023
# Get Library SO Name with Major Version
MACRO(vs_find_library _NAME _LIB)
    IF (EXISTS "${_LIB}")
        SET(VS_${_NAME}_LIB "${_LIB}")
    ELSE()
        find_library("VS_${_NAME}_LIB" NAME ${_LIB} HINTS ${X11_LIB_PATH}) # HINTS <paths>
    ENDIF()
    IF(VS_${_NAME}_LIB)
        get_filename_component(_LIB_REALPATH ${VS_${_NAME}_LIB} REALPATH)  # resolves symlinks
        get_filename_component(_LIB_BASENAME ${_LIB_REALPATH} NAME)
        IF(APPLE)
            STRING(REGEX REPLACE "(\\.[0-9]*)\\.[0-9\\.]*dylib$" "\\1.dylib" _LIB_MAJOR_NAME "${_LIB_BASENAME}")
        ELSE()
            STRING(REGEX REPLACE "(\\.[0-9]*)\\.[0-9\\.]*$" "\\1" _LIB_MAJOR_NAME "${_LIB_BASENAME}")
        ENDIF()
        SET(VS_${_NAME}_MAJOR_NAME "${_LIB_MAJOR_NAME}")
    ENDIF()
ENDMACRO()

