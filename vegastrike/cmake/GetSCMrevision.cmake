# Get the Source Control Management revision
# and output it in a C header file
# Only git is supportted right now
# Thanks to https://stackoverflow.com/questions/3780667/use-cmake-to-get-build-time-subversion-revision
# Usage:
#    ${CMAKE_COMMAND} -DGETSCM_SOURCE_DIR="<root-source-folder>" \
#                     -DGETSCM_VERSION_FILE="<output-version-file>" \
#                     -DGETSCM_PRODUCT_VERSION="<version-string>" \
#                     -P GetSCMrevision.cmake
# Example:
#    add_custom_target(get_scm_revision ALL DEPENDS get_scm_revision_output)
#    add_custom_command(OUTPUT get_scm_revision_output # trick so that make considers as PHONY and always run it
#                       COMMAND ${CMAKE_COMMAND} -DGETSCM_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
#                                                -DGETSCM_VERSION_FILE="${CMAKE_CURRENT_BINARY_DIR}/version.h"
#                                                -DGETSCM_PRODUCT_VERSION="${VERSION}"
#                                                -P "${CMAKE_CURRENT_SOURCE_DIR}/GetSCMrevision.cmake")
#    set_source_file_properties("${CMAKE_CURRENT_BINARY_DIR}/version.h" PROPERTIES GENERATED TRUE HEADER_FILE_ONLY TRUE)
#    add_dependencies(<exec_target> get_scm_revision)
#

#message ("++++++++++++++++++++ SCM SRC=${GETSCM_SOURCE_DIR} OUT=${GETSCM_VERSION_FILE}\n")

find_program(VS_GIT_BIN NAMES "git" DOC "git executable")
mark_as_advanced(VS_GIT_BIN)
unset(_SCM_REV_EXEC_STRING)
unset(_SCM_REV_EXEC_RESULT)
unset(_SCM_REMOTE_EXEC_STRING)
unset(_SCM_REMOTE_EXEC_RESULT)
unset(_SCM_VERSION_RCNUM CACHE)
if (VS_GIT_BIN)
    execute_process(COMMAND ${VS_GIT_BIN} --no-pager describe --abbrev=8 --dirty --always
        OUTPUT_VARIABLE _SCM_REV_EXEC_STRING OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_VARIABLE _SCM_REV_EXEC_ERROR ERROR_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE _SCM_REV_EXEC_RESULT
        WORKING_DIRECTORY "${GETSCM_SOURCE_DIR}"
        )
    execute_process(COMMAND ${VS_GIT_BIN} --no-pager remote get-url origin
        OUTPUT_VARIABLE _SCM_REMOTE_EXEC_STRING OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_VARIABLE _SCM_REMOTE_EXEC_ERROR ERROR_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE _SCM_REMOTE_EXEC_RESULT
        WORKING_DIRECTORY "${GETSCM_SOURCE_DIR}"
        )
endif(VS_GIT_BIN)

#message("+++++++++++++++++++ GIT ${VS_GIT_BIN} SCM ${_SCM_REV_EXEC_STRING} REMOTE ${_SCM_REMOTE_EXEC_STRING}")
if (NOT _SCM_REV_EXEC_STRING)# OR NOT _SCM_REV_EXEC_RESULT)
    set(_SCM_REV_EXEC_STRING "unknown")
endif()
if (NOT _SCM_REMOTE_EXEC_STRING)# OR NOT _SCM_REMOTE_EXEC_RESULT)
    set(_SCM_REMOTE_EXEC_STRING "unknown")
endif()
if (NOT GETSCM_PRODUCT_VERSION)
    set(GETSCM_PRODUCT_VERSION "0.0.0")
endif()
string(REGEX REPLACE "^[^0-9]*([0-9.]+).*$" "\\1" _SCM_VERSION_RCNUM "${GETSCM_PRODUCT_VERSION}")
string(REPLACE "." "," _SCM_VERSION_RCNUM "${_SCM_VERSION_RCNUM}")

# Create temporary file with obtained revision
file(WRITE "${GETSCM_VERSION_FILE}.txt" "#ifndef _CMAKE_GETSCM_VERSION_H\n# define _CMAKE_GETSCM_VERSION_H\n"
    "# define SCM_VERSION \"${GETSCM_PRODUCT_VERSION}\"\n"
    "# define SCM_VERSION_RCNUM ${_SCM_VERSION_RCNUM}\n"
    "# define SCM_REVISION \"${_SCM_REV_EXEC_STRING}\"\n"
    "# define SCM_REMOTE \"${_SCM_REMOTE_EXEC_STRING}\"\n"
    "#endif\n")

# copy to desired file only if changed
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GETSCM_VERSION_FILE}.txt" "${GETSCM_VERSION_FILE}")

