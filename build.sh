#!/bin/bash
##############################################################################
# Copyright (C) 2021-2023 Vincent Sallaberry
# vegastrike/PrivateerGold
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
##############################################################################
mydir=$(cd "$(dirname "$0")"; pwd)
mydirname=$(basename "${mydir}")
mypwd=$(pwd)

# project settings
priv_version="1.2.4"
optim_delivery="-O3"
priv_data="${mydir}/data"
target="${mydir}/vegastrike"
build_preferred="cmake configure"
build_tool=auto
build_type=RelWithDebInfo
gfx=sdl2

macos_sdk=
optim_level="${optim_delivery}"
optim_level_given=
do_delivery=
do_run=
do_debug=
do_setup=
do_dist=
do_checkpython=
interactive=
force_clean=

# default build dir/depends on gfx: ./${buildpool}/${gfx}
builddir=
buildpool=build

# prefix with bin/lib/share/... containing vega build dependencies
export VEGA_PREFIX=/usr/local/vega05
#export GTK2_PREFIX=/opt/local/
export GTK2_PREFIX=/usr/local/gtk2
unset VEGA_PREFIX2

# tools
rpath_tool="${mydir}/tools/rpath.sh"

# SYSTEM
build_sysname=$(uname -s | tr "[:upper:]" "[:lower:]")
build_sysmajor=$(uname -r | awk -F '.' '{ print $1 }')

# Compiler settings
macos_developer_dir="/Library/Developer/CommandLineTools"
unset CFLAGS CXXFLAGS LDFLAGS CPPFLAGS OBJCFLAGS OBJCPPFLAGS IPATH CPATH INCLUDE_PATH LIBRARY_PATH
#compiler=/usr/local/gcc/gcc-6.5.0_ada/bin/gcc
#compiler=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang
compiler=${CC:-clang}
cxx_standard=11
cc_flags=""
gcc_cc_flags=""
clang_cc_flags=""
#cxx_flags="-fpermissive -Wno-deprecated-declarations -Wno-unused-local-typedefs -Wno-strict-aliasing"
cxx_flags="-Wno-deprecated-declarations -Wno-unused-private-field -Wno-char-subscripts" # -Wno-strict-aliasing"
gcc_cxx_flags="-Wno-unused-local-typedefs"
clang_cxx_flags="-Wno-unused-local-typedef -Wno-deprecated-register"
cxx_ldflags=

py_version=2.7
target_libdirname="lib"

tar=$(which -a gnutar tar 2> /dev/null | head -n1)
zip=$(which -a zip 2> /dev/null | head -n1)

# ----------------------------------------------------------------------------------------------
show_help() {
    local _ret=$1
    echo "$(basename $0) "
    echo "    [-h,--help] [--tool=auto|cmake|configure] [-b,--build=<builddir>]"
    echo "    [-T,--type=<build-type] [-j,--jobs=<make-jobs>] [-G,--gfx=glut|sdl1|sdl2]"
    echo "    [--cc=<c-compiler>] [--target=<src-root-dir>] [--cxx-flags=<flags>]"
    echo "    [--cxx-std=98|11|..] [--cxx-ldflags=<flags>] [--sdk=<macos_sdk>] [-O,--optimize=-O?]"
    echo "    [-C,--clean] [-c,--clean-keep] [-i,--interactive] [--[full-]dist] [--delivery[-dev|-nodata]"
    echo "    [-s,--setup] [-r,--run] [-d,--debug[=stop]] [-D,--data=<datadir>] [-P,--{check,compile}-python]"
    echo "    [-- [<Configure/CMake args] [-- [<RUN_ARGS>]]]"
    echo ""
    echo "   compiler: ${compiler} (cxxflags: ${cxx_flags})"
    echo "  make_jobs: ${make_jobs}"
    echo "  build_dir: ${builddir:-${buildpool}/${gfx}} (absolute | [.[.]/]<path_from_cwd>)"
    echo "     target: ${target}"
    echo "       tool: ${build_tool}"
    echo " build_type: ${build_type}  (Debug|Release|NativeRelease|Maintainer|RelWithDebInfo|MinSizeRel|Profiler|...)"
    echo "  priv_data ${priv_data}"
    echo ""
    exit ${_ret}
}
parse_opts() {
    local _arg _argtype=0
    local _saveopts=; test ${#spec_config_args[@]} -eq 0 -a ${#sh_config_args[@]} -eq 0 && _saveopts=yes
    for _arg in "$@"; do
        test "${_arg}" = "--" && { _argtype=$((_argtype+1)); continue ; }
        case "${_argtype}" in
            1) test -n "${_saveopts}" && spec_config_args[${#spec_config_args[@]}]="${_arg}"
               continue ;;
            2) test -n "${_saveopts}" && run_args[${#run_args[@]}]="${_arg}"
               continue ;;
        esac
        local _ignore=yes
        case "${_arg}" in
            -h|--help)      show_help 0;;
            -b*|--build=*)  builddir=${_arg#-b};builddir=${builddir#--build=};;
            -j*|--jobs=*)   make_jobs=${_arg#-j};make_jobs=${make_jobs#--jobs=};;
            --target=*)     target=${_arg#--target=};;
            --tool=*)       _ignore=; build_tool=${_arg#--tool=};;
            --cc=*)         _ignore=; compiler=${_arg#--cc=};;
            --cxx-flags=*)  _ignore=; cxx_flags="${cxx_flags}${cxx_flags:+ }${_arg#--cxx-flags=}";;
            --cxx-ldflags=*)_ignore=; cxx_ldflags="${cxx_ldflags}${cxx_ldflags:+ }${_arg#--cxx-ldflags=}";;
            --cxx-std=*)    _ignore=; cxx_standard=${_arg#--cxx-std=};;
            -O*|--optimize=*)_ignore=; optim_level_given=yes; optim_level=${_arg#-O}; optim_level=${optim_level#--optimize=};;
            --sdk=*)        _ignore=; macos_sdk=${_arg#--sdk=};;
            -T*|--type=*)   _ignore=; build_type=${_arg#-T};build_type=${build_type#--type=};;
            --gfx=*|-G*)    _ignore=; gfx=${_arg#-G};gfx=${gfx#--gfx=};;
            --data=*|-D*)   priv_data=${_arg#-D};priv_data=$(cd "${priv_data#--data=}"; pwd);;
            --setup|-s)     do_setup=yes;;
            --run|-r)       do_run=yes;;
            --debug|-d)     do_debug=yes;;
            --debug=*|-d*)  do_debug=${_arg#-d}; do_debug=${do_debug#--debug=};;
            --check-python|-P) do_checkpython=yes;;
            --compile-python) do_checkpython=bytecode;;
            --compile-python=*) do_checkpython=bytecode:${_arg#--compile-python=};;
            --check-python=*|-P*) do_checkpython=${_arg#-P};do_checkpython=${do_checkpython#--check-python=};;
            --clean|-C)     force_clean=yes;;
            --clean-keep|-c)force_clean=a_little;;
            --interactive|-i)interactive=yes;;
            --delivery)     do_delivery=yes;;
            --delivery-dev) do_delivery=dev;;
            --delivery-nodata)do_delivery=nodata;;
            --dist)         do_dist=yes;;
            --full-dist)    do_dist=full;;
            -*) echo "error: unknown option '${_arg}'."; show_help 1;;
            *) echo "error: unknown argument '${_arg}'."; show_help 1;;
        esac
        test -z "${_ignore}" -a -n "${_saveopts}" && sh_config_args[${#sh_config_args[@]}]="${_arg}"
    done
}
yesno() {
    local _c _ret=1
    while true; do
        test -n "$@" && printf -- "$@"
        read -s -n 1 _c || return 1
        case "${_c}" in
            y|Y) _ret=0; break ;;
            n|N) _ret=1; break;;
        esac
        printf -- "${_c}" | grep -q -E '^[-0-9a-zA-Z_]$' \
        && printf -- "${_c}"
        printf -- '\n'
    done
    printf -- '\n'
    return ${_ret}
}

#
# make jobs according to cpu number
#
case "${build_sysname}" in
    linux*|cygwin*|msys*|mingw*) make_jobs=$(grep -E '^processor[[:space:]]*:' /proc/cpuinfo | wc -l);;
    *) make_jobs=$(sysctl hw.ncpu 2> /dev/null | awk '{ print $2 }');;
esac
if test -n "${make_jobs}"; then
    make_jobs=$((make_jobs))
    test ${make_jobs} -lt 4 \
        && make_jobs=$((make_jobs+1)) \
        || make_jobs=$((make_jobs-1))
else
    make_jobs=2
fi

#
# args array helpers
#
unset config_args; declare -a config_args
add_config_args() { local _arg; for _arg in "$@"; do config_args[${#config_args[@]}]="${_arg}"; done; }
add_sh_configargs() { local _arg; for _arg in "$@"; do sh_config_args[${#sh_config_args[@]}]="${_arg}"; done; }
unset sh_config_args; declare -a sh_config_args
unset spec_config_args; declare -a spec_config_args
unset run_args; declare -a run_args

#
# Parse Options
#
parse_opts "$@"

#
# Default builddir
#
test -z "${builddir}" && builddir=${buildpool}/${gfx}

#
# CHECK TARGETSOURCE, BUILDDIR, restore build options
#
target=$(cd "${target}" && pwd) || exit 1
configure="${target}/configure"

# create builddir if necessary and go into
case "${builddir}" in /*) ;; *) builddir="${mypwd}/${builddir}";; esac
test -d "${builddir}" && builddir_exists=yes || builddir_exists=
mkdir -p "${builddir}"
pushd "${builddir}" >/dev/null 2>&1 || exit 2
builddir=$(pwd); popd >/dev/null 2>&1

# build dir must not be an ancestor of source directory
case "${target}" in "${builddir}"*) echo "!! bad builddir '${builddir}'"; exit 1;; esac

build_config_cache="${builddir}/.vs_build-sh_config-cache"
# reuse previous config if not updated
if test -f "${build_config_cache}" -a \( -z "${force_clean}" -o "${force_clean}" = "a_little" \) \
&& test "${#spec_config_args[@]}" -eq 0 \
        -a "${#sh_config_args[@]}" -eq 0; then
    { _shargs=yes; while read _line; do
        test -n "${_shargs}" && { test "${_line}" = "--" && _shargs= || add_sh_configargs "${_line}"; } \
        || add_config_args "${_line}"
    done; } < "${build_config_cache}"
    parse_opts "${sh_config_args[@]}"
fi

#
# compiler detection
#
! tmp_compiler=$(which "${compiler}") \
|| ! test -x "${tmp_compiler}" \
&& { echo "error: compiler '${compiler}' not found."; exit 2; }

compiler=${tmp_compiler}

cc_dir=$(dirname "${compiler}")
compiler=$(basename "${compiler}")
case "${compiler}" in *.[eE][xX][eE]) compiler_exe=".exe"; compiler=${compiler%.exe};; *) compiler_exe=;; esac
case "${compiler}" in
    *-*-g[c+][c+]*) cc_pref="${compiler%g[c+][c+]*}gcc"; cxx_pref="${cc_pref%cc}++";;
    *-*-clang*) cc_pref="${ccpref%clang*}clang"; cxx_pref="${cc_pref}++";;
    *) cc_pref=${compiler%%-*}
       cc_pref=${cc_pref%[0-9]*}
       case "${cc_pref}" in
           [cg]"++"|cc|gcc) cc_pref=gcc; cxx_pref=g++;;
           *) cc_pref=${cc_pref%++}; cxx_pref=${cc_pref}++;;
       esac ;;
esac
cc_suff=${compiler#${cc_pref%++}}

# Identify target_sysname according to compiler
case "${cc_pref}" in
    *-*-mingw*) target_sysname=mingw; target_arch=${cc_pref%%-*};;
    *) target_sysname=${build_sysname}
       case " ${cxx_flags} " in 
           *" -m32 "*) target_arch=i386;; 
           *) target_arch=$("${tmp_compiler}" -v --version 2>&1 | sed -ne 's/^Target:[[:space:]]*\([^-]*\).*/\1/p' | head -n1)
              test -n "${target_arch}" || target_arch=$(uname -m);; 
       esac;;
esac

case "${target_arch}" in
    x86_64|arm64) target_archbits=64;;
    *) target_archbits=32;;
esac

# Exe / Shared Library file extension according to target
target_osx_version_min="10.7"
exe=
case "${target_sysname}" in
    darwin*)
        clang_cxx_flags="-stdlib=libc++${clang_cxx_flags:+ }${clang_cxx_flags}"
        build_dllext="dylib"
        if test "${target_arch}" = "arm64"; then
            target_osx_version_min="11.0"
        elif test ${build_sysmajor} -ge 19; then
            target_osx_version_min="10.11"
        fi
        ;;
    mingw*|cygwin*|msys*)
        build_dllext="dll.a"; exe=".exe"; compiler_exe=".exe"
        test "${target_archbits}" = "32" \
        && { clang_cxx_flags="-stdlib=libstdc++${clang_cxx_flags:+ }${clang_cxx_flags}"; target_libdirname="lib32"; } \
        || clang_cxx_flags="-stdlib=libc++${clang_cxx_flags:+ }${clang_cxx_flags}"
        ;;
    linux*)
        if test "${target_arch}" = "i386"; then
            export VEGA_PREFIX="${VEGA_PREFIX}-32" GTK2_PREFIX="${GTK2_PREFIX}-32}" VEGA_PREFIX2="${VEGA_PREFIX2:+${VEGA_PREFIX2-32}}"
            target_libdirname="lib32"
        fi
        # make the rpath long because chrpath cannot extend length
        cxx_ldflags="${cxx_ldflags:+ }-Wl,-rpath,${VEGA_PREFIX}/lib -Wl,-rpath,${VEGA_PREFIX}/lib/${target_arch}-linux-gnu -Wl,-rpath,${GTK2_PREFIX}/lib -Wl,-rpath,${GTK2_PREFIX}/lib/${target_arch}-linux-gnu -Wl,-rpath,/lib/${target_arch}-linux-gnu -Wl,-rpath,/usr/lib/${target_arch}-linux-gnu -Wl,-rpath,/lib${target_archbits} -Wl,-rpath,/usr/lib${target_archbits} -Wl,-rpath,/usr/lib -Wl,-rpath,/usr/local/lib -Wl,-rpath,/opt/local/lib -Wl,-rpath,/usr/local/local2/local3/local4/local5/lib"
        build_dllext="so";;
    *)
        build_dllext="so";;
esac

# specific FLAGS for clang or gcc
case "${compiler}" in
    clang*|*-clang*) spec_cc_flags="${clang_cc_flags}"; spec_cxx_flags="${clang_cxx_flags}"
            ;;
    *) spec_cc_flags="${gcc_cc_flags}"; spec_cxx_flags="${gcc_cxx_flags}"
            ;;
esac

export CC="${cc_dir}/${cc_pref}${cc_suff}${compiler_exe}"
export CFLAGS="${spec_cc_flags} ${cc_flags}"
export CXX="${cc_dir}/${cxx_pref}${cc_suff}${compiler_exe}"
export CXXFLAGS="${spec_cxx_flags} ${cxx_flags}"

cxxstdlib=$(echo " ${CXXFLAGS} " | sed -ne 's/.* \(-stdlib=[^[:space:]]*\) .*/\1/p')
test -n "${cxxstdlib}" && cxx_ldflags="${cxx_ldflags}${cxx_ldflags:+ }${cxxstdlib}"
test -n "${cxx_ldflags}" && export LDFLAGS="${cxx_ldflags}"

fix_arch_flags() {
    local _var=$1; shift
    eval echo "\${$_var}}" | grep -Eq '\(^| \)-arch ' && return 0
    while test -n "$1"; do
        case "$1" in -arch) shift; eval "${_var}=\"\${${_var}}\${${_var}:+ }-arch $1\"";; esac
    shift; done
}

get_archs() {
    while test -n "$1"; do
        case "$1" in 
            -arch) shift; echo "$1";; 
            -m32) case "${target_arch}" in x86*|i[0-9]86*) echo "i386";; esac;;
            -m64) case "${target_arch}" in x86*|i[0-9]86*) echo "x86_64";; esac;;
        esac
    shift; done | sort | uniq
}

fix_arch_flags CFLAGS ${CXXFLAGS}
fix_arch_flags LDFLAGS ${CXXFLAGS}

cxx_standard_autoconf="gnu++${cxx_standard}"

#
#### FIND SDK
#
find_macos_sdk() {
    local _cxxflags
    test -z "$1" && _cxxflags=${CXXFLAGS} || _cxxflags=$1
    _test_sdk=$(${CC} -v --version 2>&1 | sed -n -e 's%^.*[[:space:]=:]\(/[^[:space:]]*/SDKs/MacOSX[0-9.][0-9.]*\.sdk\).*$%\1%p' | head -n1)
    sysver=$(uname -r)
    if test ${sysver%%.*} -gt 15; then # 15 is capitan
        # sierra or later
        if test -n "${_test_sdk}"; then
            macos_sdk_fwk=${_test_sdk}
        else
            archs=$(get_archs ${_cxxflags})
            macos_sdk_fwk="${macos_developer_dir}/SDKs/MacOSX.sdk"
            if test -n "${archs}"; then
                # try to find a SDK supporting requested archs
                for sdk in "${macos_developer_dir}/SDKs"/MacOSX*.sdk; do
                    if lipo $(find "${sdk}/usr/lib" -name '*.o' -o -name '*.a' | head -n1) -verify_arch ${archs}; then
                        macos_sdk_fwk=${sdk}
                        break
                    fi
                done
            fi
        fi
        macos_sdk=${macos_sdk_fwk}
    else
        # capitan or earlier
        #macos_sdk_fwk="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk"
        macos_sdk_fwk="${macos_developer_dir}/SDKs/MacOSX.sdk"
        macos_sdk=""
    fi
}
if test -n "${macos_sdk}"; then
    macos_sdk_fwk=${macos_sdk}
else
    find_macos_sdk
fi

case "${target_sysname}" in
    mingw*)
        case "${target_arch}" in "x86_64") asuff=64;; arm64) suff=arm64;; *) asuff=32;; esac
        case "${cc_pref}" in *clang*) test "${asuff}" = 32 && suff="mingw" || suff="clang";; *) suff="mingw";; esac
        export VEGA_PREFIX="/${suff}${asuff}"; export GTK2_PREFIX="${VEGA_PREFIX}"; export VEGA_PREFIX2="/usr/local${asuff}"
        test "${asuff}" = "32" && export WIN32LIBS_PREFIX=/mingw32 || WIN32LIBS_PREFIX="/${suff}${asuff}/${target_arch}-w64-mingw32"
        ;;
esac

# EMBEDED PYTHON
case "${target_sysname}" in
    darwin*)
        export PYTHON_PREFIX="${VEGA_PREFIX}/Library/Frameworks/Python.framework/Versions/${py_version}"
        export PYTHON="${PYTHON_PREFIX}/bin/python${py_version}"
        export PYTHON_LIBRARY="${PYTHON_PREFIX}/lib/libpython${py_version}.dylib"
        export PYTHON_INCLUDE_DIR="${PYTHON_PREFIX}/include/python${py_version}";;
    *)
        export PYTHON_PREFIX="${VEGA_PREFIX}"
        export PYTHON="${PYTHON_PREFIX}/bin/python${py_version}"
        export PYTHON_LIBRARY="${PYTHON_PREFIX}/lib/libpython${py_version}.${build_dllext}"
        export PYTHON_INCLUDE_DIR="${PYTHON_PREFIX}/include/python${py_version}"
        case "${target_sysname}" in mingw*|cygwin*|msys*) export PYTHON="$(cygpath -m "${PYTHON}")";; esac
        ;;
esac

# PATH, tools, dependencies settings
export GTK2_HOME="${GTK2_PREFIX}/include/gtk-2.0"
export PATH="/usr/bin:/bin:/usr/sbin:/sbin:${VEGA_PREFIX2:+${VEGA_PREFIX2}/bin:${VEGA_PREFIX2}/sbin:}${VEGA_PREFIX}/bin:${VEGA_PREFIX}/sbin:${GTK2_PREFIX}/bin:${GTK2_PREFIX}/sbin:/opt/local/bin:/opt/local/sbin:/usr/local/gcc/gcc-6.5.0_ada/bin:/usr/local/bin"
#export PATH="${VEGA_PREFIX}/bin:/usr/bin:/bin:/usr/sbin:/sbin:${VEGA_PREFIX}/sbin:/opt/local/bin:/opt/local/sbin"
export MAKE=$(which -a gmake make 2> /dev/null | head -n 1)
export CMAKE=$(which -a cmake 2> /dev/null | head -n1)

export PKGCONFIG="${VEGA_PREFIX}/bin/pkg-config"
export PKG_CONFIG="${PKGCONFIG}"
PKG_CONFIG_PATH="${VEGA_PREFIX2:+${VEGA_PREFIX2}/lib/pkgconfig:}${VEGA_PREFIX}/lib/pkgconfig"
case "${build_sysname}" in linux*) PKG_CONFIG_PATH+=":${VEGA_PREFIX}/lib/${target_arch}-linux-gnu/pkgconfig";; esac
PKG_CONFIG_PATH+=":${PYTHON_PREFIX}/lib/pkgconfig"
PKG_CONFIG_PATH+=":${GTK2_PREFIX}/lib/pkgconfig"
case "${build_sysname}" in linux*) PKG_CONFIG_PATH+=":${GTK2_PREFIX}/lib/${target_arch}-linux-gnu/pkgconfig:${GTK2_PREFIX}/share/pkgconfig";; esac
PKG_CONFIG_PATH+=":/usr/lib/pkgconfig"
PKG_CONFIG_PATH+=":/opt/local/lib/pkgconfig"
export PKG_CONFIG_PATH

LIBPNG_INCLUDE_DIR=$("${PKGCONFIG}" --cflags libpng | sed -e 's/^[[:space:]]*-I\([^[:space:]]*\).*/\1/')
test -z "${LIBPNG_INCLUDE_DIR}" && LIBPNG_INCLUDE_DIR="${VEGA_PREFIX}/include"

do_build_fun() {
    #
    # CHECK BUILDDIR, detect build_tool, clean if requested
    #
    # AUTODETECT BUILD_TOOL according to current build directory
    if test -n "${builddir_exists}"; then
        if test "${build_tool}" = "auto"; then
            { test -f "${builddir}/config.log" && build_tool=configure; } \
            || { test -d "${builddir}/CMakeFiles" && build_tool=cmake; }
        fi
    fi
    # AUTODETECT BUILD_TOOL according to CMakeLists.txt/configure.ac presence, and a priority list
    if test "${build_tool}" = "auto"; then
        build_tools=
        test -f "${target}/CMakeLists.txt" && build_tools="${build_tools}${build_tools:+ }cmake"
        test -f "${target}/configure.ac" && build_tools="${build_tools}${build_tools:+ }configure"
        for b in ${build_preferred}; do
            if echo " ${build_tools} " | grep -Eq " ${b} "; then
                build_tool=${b};
                break ;
            fi
        done
        test "${build_tool}" = "auto" && build_tool=${build_preferred%% *}
    fi

    case "${build_type}" in
        [rR][eE][aAlL]*|[nN][aA][tT]*) _p_optim=${optim_level};;
        *) _p_optim=;;
    esac
    #
    #### START CONFIG & BUILD
    #
    echo "+ CC        ${CC}  (${target_arch}) (${CFLAGS})"
    echo "+ CXX       ${CXX}  (std:${cxx_standard}, ${CXXFLAGS}, ldflags: ${LDFLAGS})"
    echo "+ PATH      ${PATH}"
    echo "+ PKGCONFIG ${PKG_CONFIG}: ${PKG_CONFIG_PATH}"
    echo "+ SDK       ${macos_sdk}"
    echo "+ SDK_FWK   ${macos_sdk_fwk}"
    echo "+ SRC       ${target}"
    echo "+ BUILDDIR  ${builddir}"
    echo "+ BUILDER   ${build_tool}"
    echo "+ BUILDTYPE ${build_type}"
    echo "+ GFX       ${gfx}"
    test -n "${_p_optim}" && echo "+ OPTIM     ${_p_optim}"
    echo

    # Clean builddir if requested
    test -n "${builddir_exists}" \
        && test -n "${interactive}" -o -n "${force_clean}" && yesno "? ${builddir} exists. Remove it ? (y: remove, n: keep) " \
        && rm -Rf "${builddir}" && { mkdir -p "${builddir}" || exit 2; } && echo "+ ${builddir} deleted."

    ### FIND LIBS
    for f in "pkg:gtk+-2.0"; do
        case "$f" in
            pkg:*) test_libs=`pkg-config --libs "${f#pkg:}"`;;
            *) test_libs="${f}";;
        esac
        libpath=; libs=; for lib in ${test_libs}; do
            case "${lib}" in
                -L*) libpath="${lib#-L}"; libs="${libs}${libs:+ }${lib}";;
                -l*) test -f "${libpath}/lib${lib#-l}.a" && libs="${libs}${libs:+ }${libpath}/lib${lib#-l}.a" \
                     || { echo "!! static '${lib}' not found"; libs="${libs}${libs:+ }${lib}";};;
                *) libs="${libs}${libs:+ }${lib}";;
            esac
        done; echo "** Satic libs: "; echo "${libs}"
    done

    ### Update source build link (for IDEs: they need config.h and other stuff)
    target_build="${target}/build"
    if test -L "${target_build}" -o ! -r "${target_build}"; then
        #if test "$(readlink "${target_build}")" != "${builddir}"; then
        if ! test -f "${target_build}/config.h"; then
            rm -f "${target_build}"
            ln -sv "${builddir}" "${target_build}"
        fi
    fi

    ### CONFIG: CMAKE, configure
    echo
    echo "+ Build (${build_tool})"
    # go in builddir
    pushd "${builddir}" >/dev/null 2>&1 || exit 2

    configure_if_needed() {
        local _configure_cmd=$1
        shift
        printf -- "%s\n" "${sh_config_args[@]}" "--" "$@" | diff -u -q "${build_config_cache}" - 2> /dev/null \
            && test -f Makefile \
            || { echo "+ build config changed"; echo "${_configure_cmd} \\"; printf -- '  %s \\\n' "$@"
                 echo; test -z "${interactive}" || yesno "? configure ?" \
                 && "${_configure_cmd}" "$@" \
                 && printf -- "%s\n" "${sh_config_args[@]}" "--" "$@" > "${build_config_cache}"; }
    }

    test "${cxx_standard}" = 98 && boost_internal="1_35" || boost_internal="1_50"

    case "${build_tool}" in
        cmake)
            #export SDLDIR=${VEGA_PREFIX}
            #SDL_BUILDING_LIBRARY # if set no sdlmain is added
            #-DZLIB_ROOT=/usr \
            if test ${#config_args[@]} -eq 0; then
                add_config_args \
                -DCMAKE_CXX_COMPILER="${CXX}" \
                -DCMAKE_C_COMPILER="${CC}" \
                ${cmake_generator:+"-G${cmake_generator}"} \
                -DCMAKE_VERBOSE_MAKEFILE=ON \
                -DCMAKE_FIND_FRAMEWORK=LAST \
                -DBOOST_ROOT="${VEGA_PREFIX}/libexec/boost/150" \
                -DBOOST_INTERNAL="${boost_internal}" \
                -D_GNU_SOURCE=1 \
                -DHAVE_TR1_UNORDERED_MAP=0 \
                -DCMAKE_BUILD_TYPE="${build_type}" \
                -DSDL_DISABLE=0 \
                -DCMAKE_CXX_STANDARD="${cxx_standard}" \
                -DVS_STATIC_LIBS=0 \
                -DVS_OPTIMIZE="${optim_level}" \
                -DVS_DEBUG_LEVEL="-g3" \
                -DPYTHON_LIBRARY="${PYTHON_LIBRARY}" -DPYTHON_INCLUDE_DIR="${PYTHON_INCLUDE_DIR}"

                test "${target_libdirname}" != "lib" && add_config_args -DVEGASTRIKE_LIBDIR_NAME="${target_libdirname}"

                #rpaths="${VEGA_PREFIX}/lib;${GTK2_PREFIX}/lib"
                #add_config_args -DCMAKE_INSTALL_RPATH="${rpaths}" -DCMAKE_BUILD_RPATH="${rpaths}"
                #
                #;${VEGA_PREFIX}/lib/libvorbisenc.${build_dllext}

                case "${target_sysname}" in
                    darwin*)
                        export OPENALDIR="${VEGA_PREFIX}"
                        add_config_args \
                            -DCMAKE_OSX_SYSROOT="${macos_sdk}" \
                            -DCMAKE_OSX_DEPLOYMENT_TARGET="${target_osx_version_min}" \
                            -DZLIB_INCLUDE_DIR=${VEGA_PREFIX}/include -DZLIB_LIBRARY=${VEGA_PREFIX}/lib/libz.${build_dllext} \
                            -DBZ2_INCLUDE_DIR=${VEGA_PREFIX}/include -DBZ2_LIBRARY=${VEGA_PREFIX}/lib/libbz2.${build_dllext}  \
                            -DPNG_INCLUDE_DIRS=${LIBPNG_INCLUDE_DIR} -DPNG_LIBRARIES=${VEGA_PREFIX}/lib/libpng.dylib \
                            -DJPEG_INCLUDE_DIR=${VEGA_PREFIX}/include -DJPEG_LIBRARY=${VEGA_PREFIX}/lib/libjpeg.dylib \
                            -DVorbis_INCLUDE_DIRS=${VEGA_PREFIX}/include \
                            -DVorbis_LIBRARIES="${VEGA_PREFIX}/lib/libvorbisfile.dylib;${VEGA_PREFIX}/lib/libvorbis.dylib;${VEGA_PREFIX}/lib/libogg.dylib" \
                            -DVS_FIND_PREFIX_MORE_PATHS="${VEGA_PREFIX} ${GTK2_PREFIX}" \
                            -DGLUT_INCLUDE_DIR="${macos_sdk_fwk}/System/Library/Frameworks/GLUT.framework/Headers/" \
                            -DGLUT_LIBRARIES="${macos_sdk_fwk}/System/Library/Frameworks/GLUT.framework/GLUT.tbd" \
                            -DOPENAL_INCLUDE_DIR="${VEGA_PREFIX}/include/AL" -DOPENAL_LIBRARY="${VEGA_PREFIX}/lib/libopenal.dylib"
                            #-DOPENAL_LIBRARY="${macos_sdk_fwk}/System/Library/Frameworks/OpenAL.framework/Versions/A/OpenAL.tbd" \
                            #-DOPENAL_INCLUDE_DIR="${macos_sdk_fwk}/System/Library/Frameworks/OpenAL.framework/Versions/A/Headers"
                            #-DZLIB_INCLUDE_DIR=${macos_sdk}/usr/include -DZLIB_LIBRARY=/usr/lib/libz.${build_dllext}
                            #-DBZ2_INCLUDE_DIR=${macos_sdk}/usr/include -DBZ2_LIBRARY=/usr/lib/libbz2.${build_dllext}

                        ;;
                    mingw*)
                        case "$(uname -o | tr "[:upper:]" "[:lower:]")" in
                            msys*) add_config_args -G"MSYS Makefiles";;
                            *) add_config_args -G"MinGW Makefiles";;
                        esac
                        case "${target_arch}" in i[0-9]86|x86) 
                            add_config_args -DGLUT_LIBRARIES="${VEGA_PREFIX2}/lib/libglut.dll.a" 
                                            -DGLUT_INCLUDE_DIR="${VEGA_PREFIX2}/include/GL";; 
                        esac
                        #-DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
                        add_config_args \
                            -DMATH_INCLUDE_DIRS="${VEGA_PREFIX}/${target_arch}-w64-mingw32/include" \
                            -DMATH_LIBRARIES="${WIN32LIBS_PREFIX}/lib/libm.a" \
                            -DPNG_INCLUDE_DIRS="${LIBPNG_INCLUDE_DIR}" \
                            -DPNG_LIBRARIES="${VEGA_PREFIX}/lib/libpng.dll.a" \
                            -DSHLWAPI_LIB="${WIN32LIBS_PREFIX}/lib/libshlwapi.a" \
                            -DCMAKE_MAKE_PROGRAM="${MAKE}"
                            #-DPNG_INCLUDE_DIRS="${VEGA_PREFIX2}/include/libpng12" \
                            #-DPNG_LIBRARIES="${VEGA_PREFIX2}/lib/libpng.a"
                        ;;
                    *)
                        case " ${cxx_flags} " in *" -m32 "*) test "$(uname -m)" != "i386" \
                            && add_config_args -DCMAKE_PREFIX_PATH="${VEGA_PREFIX};/usr/local/linux-gl32";; 
                        esac
                        add_config_args \
                            -DPNG_INCLUDE_DIRS=${LIBPNG_INCLUDE_DIR} -DPNG_LIBRARIES=${VEGA_PREFIX}/lib/libpng.so \
                            -DJPEG_INCLUDE_DIR=${VEGA_PREFIX}/include -DJPEG_LIBRARY=${VEGA_PREFIX}/lib/libjpeg.so \
                            -DVorbis_INCLUDE_DIRS=${VEGA_PREFIX}/include \
                            -DVorbis_LIBRARIES="${VEGA_PREFIX}/lib/libvorbisfile.so;${VEGA_PREFIX}/lib/libvorbis.so;${VEGA_PREFIX}/lib/libogg.so" \
                            -DVS_FIND_PREFIX_MORE_PATHS="${VEGA_PREFIX} ${GTK2_PREFIX}" \
                            -DOPENAL_INCLUDE_DIR="${VEGA_PREFIX}/include" -DOPENAL_LIBRARY="${VEGA_PREFIX}/lib/libopenal.so" \
                            -DGLUT_INCLUDE_DIR="${VEGA_PREFIX}/include" -DGLUT_LIBRARIES="${VEGA_PREFIX}/lib/libglut.so" \
                            -DZLIB_INCLUDE_DIR="${VEGA_PREFIX}/include" -DZLIB_LIBRARIES="${VEGA_PREFIX}/lib/libz.so" \
                            -DBZ2_INCLUDE_DIR="${VEGA_PREFIX}/include" -DBZ2_LIBRARIES="${VEGA_PREFIX}/lib/libbz2.so"
                        ;;
                esac

                export SDLDIR=${VEGA_PREFIX}
                case "${gfx}" in
                    glut1) add_config_args "-DSDL_WINDOWING_DISABLE=1";;
                    glut) add_config_args "-DSDL_WINDOWING_DISABLE=1" \
                                          -DSDL_INCLUDE_DIR="${VEGA_PREFIX}/include/SDL2" \
                                          -DSDL_LIBRARY="${VEGA_PREFIX}/lib/libSDL2.${build_dllext}";;
                    sdl1)
                        case "${target_sysname}" in
                            mingw*|cygwin*|msys*)
                                case "${target_arch}" in *64) export SDLDIR="/usr/local64";; esac
                                add_config_args "-DSDL_WINDOWING_DISABLE=0" \
                                    -DSDL_INCLUDE_DIR="${SDLDIR}/include/SDL" \
                                    -DSDL_LIBRARY="${SDLDIR}/lib/libSDLmain.a;${SDLDIR}/lib/libSDL.${build_dllext}";;
                            *)
                                add_config_args "-DSDL_WINDOWING_DISABLE=0" \
                                  -DSDL_INCLUDE_DIR="${VEGA_PREFIX}/include/SDL";;
                                  #-DSDL_LIBRARY="${VEGA_PREFIX}/lib/libSDLmain.a;${VEGA_PREFIX}/lib/libSDL.${build_dllext}";;
                        esac;;
                    sdl2) add_config_args -DSDL_WINDOWING_DISABLE=0 \
                                          -DSDL_INCLUDE_DIR="${VEGA_PREFIX}/include/SDL2" \
                                          -DSDL_LIBRARY="${VEGA_PREFIX}/lib/libSDL2main.a;${VEGA_PREFIX}/lib/libSDL2.${build_dllext}";;
                    *) echo "!! unknown gfx '${gfx}'"; exit 1;;
               esac
               add_config_args \
                "${spec_config_args[@]}" \
                "${target}"
            fi

            configure_if_needed "${CMAKE}" "${config_args[@]}"

            ;;
        configure)
            case "$(echo "${build_type}" | tr "[:upper:]" "[:lower:]")" in
                release) debug_config_args="--disable-debug --enable-release=${optim_level#-O}";;
                nativerelease) debug_config_args="--disable-debug --enable-release=${optim_level#-O}";;
                debug) debug_config_args="--enable-debug";;
                maintainer) debug_config_args="--enable-debug";;
                relwithdebinfo) debug_config_args="--enable-debug --enable-release=${optim_level#-O}";;
                profiler) debug_config_args="--enable-debug --enable-profile";;
                *) debug_config_args="--enable-debug --enable-release=${optim_level#-O}";;
            esac
            ! test -f "config.log" \
            || test "${target}/configure.ac" -nt "${target}/configure" \
            || for f in "${target}"/m4scripts/*; do
                test "${f}" -nt "${target}/configure" && break
            done \
            && (rm -f Makefile; cd "${target}" && ./bootstrap-sh || exit 3)
            #./configure   LDFLAGS=-L/Users/danielrh/Vega/lib
            #--with-expat-libs=/Users/danielrh/Vega/lib
            #--with-expat-inc=/Users/danielrh/Vega/include/
            #--with-png-libs=/Users/danielrh/Vega/lib
            #--with-png-inc=/Users/danielrh/Vega/include/
            #--with-jpeg-inc=/Users/danielrh/Vega/include #
            #--with-jpeg-libs=/Users/danielrh/Vega/lib
            #--with-ogg-inc=/Users/danielrh/Vega/include
            #--with-ogg-libs=/uSers/danielrh/Vega/lib
            #--with-vorbis-inc=/Users/danielrh/Vega/include
            #--with-vorbis-libs=/Users/danielrh/Vega/lib
            #--with-sdl-prefix=/Users/danielrh/Vega/ #
            #--with-sdl-exec-prefix=/Users/danielrh/Vega/
            #--with-python-inc=/Users/danielrh/Vega/Python-2.4.4/Include
            #--with-python-libs=/Users/danielrh/Vega/Python-2.4.4/
            #--enable-macosx-bundle $*

            #--with-ogg-inc=/opt/local/include --with-ogg-libs=/opt/local/lib \
            #--with-expat-libs=/opt/local/lib --with-expat-inc=/opt/local/include/ \
            #--disable-unordered-map
            boost_internal=$(echo "${boost_internal}" | sed -e 's/_/./g')
            
            case "${gfx}" in
                glut) export SDL_CONFIG="${VEGA_PREFIX}/bin/sdl2-config";;
                glut1|sdl1) export SDL_CONFIG="${VEGA_PREFIX}/bin/sdl-config";;
                sdl2) export SDL_CONFIG="${VEGA_PREFIX}/bin/sdl2-config"
            esac

            if test ${#config_args[@]} -eq 0; then
                add_config_args \
                --with-png-inc="${VEGA_PREFIX}/include" --with-png-libs="${VEGA_PREFIX}/lib" \
                --with-ffmpeg-inc="${VEGA_PREFIX}/include" --with-ffmpeg-libs="${VEGA_PREFIX}/lib" \
                --with-jpeg-inc="${VEGA_PREFIX}/include" --with-jpeg-libs="${VEGA_PREFIX}/lib" \
                --enable-sdl \
                --with-sdl-prefix="${VEGA_PREFIX}" --with-sdl-exec-prefix="${VEGA_PREFIX}" \
                --with-vorbis-inc="${VEGA_PREFIX}/include" --with-vorbis-libs="${VEGA_PREFIX}/lib" \
                --with-python="${PYTHON}" \
                --with-python-inc="${PYTHON_INCLUDE_DIR}" --with-python-libs="$(dirname "${PYTHON_LIBRARY}")" \
                --with-boost="${boost_internal}" \
                --with-cxx-std="${cxx_standard_autoconf}" \
                ${debug_config_args}

                test "${target_libdirname}" != "lib" && add_config_args --with-vegastrike-libdir-name="${target_libdirname}"

                case "${gfx}" in
                    glut) add_config_args "--disable-sdl-windowing" --with-sdl=2.0.0 --with-sdl-inc="${VEGA_PREFIX}/include/SDL2" \
                                                                    --with-sdl-libs="-L${VEGA_PREFIX}/lib -lSDL2";;
                    glut1) add_config_args --disable-sdl-windowing;;
                    sdl1) add_config_args --enable-sdl-windowing;;
                    sdl2) add_config_args "--enable-sdl-windowing" --with-sdl=2.0.0 --with-sdl-inc="${VEGA_PREFIX}/include/SDL2" \
                                                                   --with-sdl-libs="-L${VEGA_PREFIX}/lib -lSDL2main -lSDL2";;
                    *) echo "!! unknown gfx '${gfx}'"; exit 1;;
                esac

                case "${target_sysname}" in
                    darwin*)
                        add_config_args \
                            --with-macos-sdk="${macos_sdk_fwk}" \
                            --enable-macosx-bundle \
                            --with-macos-deployment-target="${target_osx_version_min}" \
                            --with-al-inc="${VEGA_PREFIX}/include/AL" --with-openal-libs="${VEGA_PREFIX}/lib"
                        ;;
                    mingw*|cygwin*|msys*)
                        case "${target_arch}" in 
                            i[0-9]86|x86) glut_prefix=${VEGA_PREFIX2};; 
                            *) glut_prefix=${VEGA_PREFIX};;
                        esac 
                        add_config_args \
                            --with-al-inc="${VEGA_PREFIX}/include" --with-openal-libs="${VEGA_PREFIX}/lib" \
                            --with-glut-inc="${glut_prefix}/include/GL" --with-glut-libs="${glut_prefix}/lib"
                        ;;
                    *)
                        add_config_args \
                            --with-al-inc="${VEGA_PREFIX}/include" --with-openal-libs="${VEGA_PREFIX}/lib" \
                            --with-glut-inc="${VEGA_PREFIX}/include" --with-glut-libs="${VEGA_PREFIX}/lib"
                        ;;
                esac

                if test "${target_sysname}" != "${build_sysname}"; then
                    add_args --build="$(uname -m)-${build_sysname}" --host="${target_arch}-${target_sysname}"
                fi

                add_config_args \
                    "${spec_config_args[@]}"
            fi

            test -f Makefile || echo '--' >> ${build_config_cache}

            configure_if_needed "${configure}" "${config_args[@]}"

            ;;
        *) echo "!! error: unknown build tool '${build_tool}'."; exit 1;;
    esac
    test $? -eq 0 || exit 3

    # necessary for vssetup to run
    #echo '.privgold100' >> setup/Version.txt
    cp "${priv_data}/Version.txt" setup

    ### BUILD : MAKE
    test -n "${interactive}" && { yesno "? Build? " || exit 4; }

    ${MAKE} VERBOSE=1 -j${make_jobs}
    ret=$?

    if false && test $ret -ne 0 -a -f CMakeFiles/vegastrike.dir/link.txt; then
        #linker="\\1"                       #use same linker
        linker="clang++ -stdlib=libstdc++"  #use clang++
        eval `cat CMakeFiles//vegastrike.dir/link.txt \
            | sed -e 's%^[[:space:]]*\([^[:space:]]*\)\(.*\)%'"${linker}"' -v \2 \1%' \
                  -e 's%\([^[:space:]]\)/[^/]*$%\1/../lib/libstdc++.a%'`
        ret=$?
    fi

    return $ret
}

do_run_fun() {
    if test -n "${do_run}" -o -n "${do_debug}" -o -n "${do_setup}"; then
        local ret=0 build data delay log args a
        local -a gdbargs
        build=$(cd "${builddir}";pwd)
        data=$(cd "${priv_data}";pwd)
        delay=20
        log="${build}/log"
        mkdir -p $(dirname "${log}")
        while test -d "${log}"; do log="${log}_"; done
        mkdir -p "${mydir}/logs"
        rm -f "${mydir}/logs/log"
        ln -s "${log}" "${mydir}/logs/log"

        echo "BUILD $build DATA $data ARGS ${run_args[@]}"

        case "${build_sysname}" in
            linux*|*bsd*) export LD_LIBRARY_PATH="${VEGA_PREFIX}/lib:${VEGA_PREFIX}/lib/${target_arch}-linux-gnu:${GTK2_PREFIX}/lib:${GTK2_PREFIX}/lib/${target_arch}-linux-gnu:${LD_LIBRARY_PATH:+:}${LD_LIBRARY_PATH}";;
            mingw*|cygwin*|msys*)
                export PATH="${VEGA_PREFIX2:+${VEGA_PREFIX2}/${target_arch}-w64-mingw32/bin:${VEGA_PREFIX2}/bin:}${VEGA_PREFIX}/bin:${GTK2_PREFIX}/bin:${PATH}:/mingw${target_archbits}/bin:/mingw${target_archbits}/${target_arch}-w64-mingw32/bin";;
        esac

        # important the chdir to have music and avoid crash when loading universe
        pushd "${data}" || exit 1

        if test -n "${do_setup}"; then
            # SETUP
            "${build}/setup/vssetup${exe}" "${run_args[@]}" || "${build}/setup/vssetup_dlg${exe}" "${run_args[@]}" \
            || "${build}/vssetup${exe}" "${run_args[@]}" || "${build}/vssetup_dlg${exe}" "${run_args[@]}"; ret=$?
        elif test -n "${do_debug}"; then # -D${data}
            # GDB
            args="${run_args[@]}"
            test "${do_debug}" = "stop" && for a in "--one-line-on-crash" 'thread backtrace all' "--one-line-on-crash" 'quit'; do gdbargs[${#gdbargs[@]}]=$a; done \
            || args="${args}${args:+ }-Cgraphics/fullscreen=false"
            ( unset PYTHON PYTHONHOME PYTHONPATH; lldb "${build}/vegastrike${exe}" \
               --batch -o "run -Clog/colorize=yes -Clog/file=stderr ${args}" "${gdbargs[@]}" 2>&1;) | tee "${log}"

            #{ { "${build}/vegastrike" --batch -o "run" 2>&1 >&3; } 3>&2 | tee "${log}"

            #{ printf -- "run -D${data}\nbt\n"; sleep ${delay}; printf -- 'bt\n'; sleep 2; exit; } \
            #  |  { unset PYTHON PYTHONHOME PYTHONPATH; lldb "${build}/vegastrike" 2>&1; } | tee "${mydir}/${log}" & privpid=$!; ret=$?
        else
            # RUN vegastrike
            case "${target_sysname}" in msys*|cygwin*|mingw*) ;; *) touch -t 197001010000 "${log}";; esac
            #"${build}/vegastrike${exe}" "-Clog/file=${log}" "-Clog/colorize=yes" "${run_args[@]}" 2>&1 | tee "${log}"; ret=$?
            ("${build}/vegastrike${exe}" "-Clog/file=${log}" "-Clog/colorize=yes" "${run_args[@]}" & vegapid=$!
             tail -F "${log}" & tailpid=$!
             wait ${vegapid}; ret=$?; kill ${tailpid})
        fi

        popd >/dev/null #data
        return $ret
    fi
}
### do_convert
###for f in ~/.privgold100/save/*; do iconv -c -f ISO8859-1 -t UTF-8 $f > $f.tmp && mv -v $f.tmp $f; done
###

#
### DIST / make tarball
#
do_dist_fun() {
    local what=$1
    local ret=0

    now_fmt=`date '+%Y.%m.%d_%Hh%Mm%Ss'`
    archive="${mydirname}_${now_fmt}.tar.xz"
    format_size() {
        local size=`stat -f '%z' "$@"`
        if test -z "$size"; then
           echo 0
        elif test $size -gt 1000000000000; then
            printf -- '%0.3gT' $((size/1000000000000))
        elif test $size -gt 1000000000; then
            printf -- '%0.3gG' $((size/1000000000))
        elif test $size -gt 1000000; then
            printf -- '%0.3gM' $((size/1000000))
        elif test $size -gt 1000; then
            printf -- '%0.3gK' $((size/1000))
        else
            printf -- '%0.3gB' ${size}
        fi
    }

    pushd "${mydir}/.." >/dev/null 2>&1

    echo "+ creating archive '${archive}' from '${mydir}/../${mydirname}'"

    local tar_args
    unset tar_args; declare -a tar_args
    for arg in  \
        --exclude "${mydirname}/vegastrike*/build" \
        --exclude "${mydirname}/vegastrike*/build-*" \
        --exclude "${mydirname}/engine*/build" \
        --exclude "${mydirname}/engine*/build-*" \
        --exclude "${mydirname}/builds" \
        --exclude "${mydirname}/build" \
        --exclude "${mydirname}/release" \
        --exclude "${mydirname}/${buildpool}" \
        --exclude "**/*.sw?" --exclude "**/*~" --exclude "**/.\#*" \
    ; do
        tar_args[${#tar_args[@]}]="${arg}"
    done
    if test "${what}" != "full"; then
        for arg in \
            --exclude "${mydirname}/tmp" \
            --exclude "**/.svn" --exclude "**/.git" --exclude "**/CVSROOT" \
        ; do
            tar_args[${#tar_args[@]}]="${arg}"
        done
    fi
    "${tar}" cJ \
        "${tar_args[@]}" \
        -f "${archive}" "${mydirname}" \
    && echo "+ archive created: ${archive} (`format_size "${archive}"`)"

    ret=$?
    popd >/dev/null

    return $ret
}

do_delivery_fun_real() {
    local deliverydir=$1
    local force_color=$2

    echo "***********  D E L I V E R Y   ********************************************"
    printf -- "+ "; date
    echo "+ SOURCE      ${target}"
    echo "+ DELIVERY    ${deliverydir}"
    echo "+ TARGET      ${target_sysname} / ${target_arch}"
    echo

    if test -n "${force_clean}"; then
        test -z "${interactive}" || yesno "? delete delivery ${deliverydir} ?" \
        && { echo "+ deleting delivery dir (${deliverydir})" \
             && rm -Rf "${deliverydir}" && mkdir -p "${deliverydir}" || exit 1; }
    fi

    if case "${do_checkpython}" in no|NO) false;; *) true;; esac; then (
        local pythonlibs_dir=
        for d in "${deliverydir}/sdl2" "${buildpool}/sdl2-${target_archbits}" "${builddir}"; do
            test -d "${d}/${target_libdirname}/pythonlibs" && { pythonlibs_dir="${d}/${target_libdirname}/pythonlibs"; break ; }
        done
        builddir="${deliverydir}/sdl2"
        do_checkpython_fun "bytecode" "${force_color}" "${pythonlibs_dir}" || exit $?
    ); fi

    pushd "${deliverydir}" > /dev/null || exit 1

    get_git_rev() {
        (cd "${target}/.." && git --no-pager describe --abbrev=8 --dirty --always || echo "unknown")
        #local git_rev=$(git --git-dir="${target}/../.git" show --quiet --ignore-submodules=untracked --format="%h" HEAD)
        #local git_status=$(git --git-dir="${target}/../.git" -C "${target}/.." status --untracked-files=no --ignore-submodules=untracked --short --porcelain)
        #test -n "${git_status}" && git_rev="${git_rev}-dirty"
        #echo "${git_rev}"
    }
    git_rev=$(get_git_rev)
    if test "${do_delivery}" != "yes" -a "${do_delivery}" != "nodata" || case "${git_rev}" in *-dirty) true;; *) false;; esac; then
        bundle_version="${priv_version}-${git_rev}"
    else
        bundle_version=${priv_version}
    fi

    bundle_tools="${mydir}/tools/macos_bundle"
    bundle_src="${bundle_tools}/PrivateerGold.app"
    checkkeys_dir="${bundle_tools}/checkModifierKeys"
    hardshortcut_dir="${mydir}/tools/hardshortcut"

    bundle_basename="PrivateerGold-${bundle_version}"
    bundle_pkgbasename="${bundle_basename}"
    this_bundle="${deliverydir}/bundle/PrivateerGold.this.app"
    other_bundle="${deliverydir}/bundle/PrivateerGold.more-archs/${bundle_basename}"
    other_bundle_ref_dir="${mydir}/${buildpool}"

    xcopy() {
        rsync -ah -t --exclude '.DS_Store' --exclude '*~' --exclude '.*.sw?' --exclude '._*' \
                     --exclude '**/.git' --exclude '**/.svn' "$@"
    }

    unset rpath_excludes; declare -a rpath_excludes
    add_rpath_excludes() { local _arg; for _arg in "$@"; do rpath_excludes[${#rpath_excludes[@]}]="-X${_arg}"; done; } 
    add_rpath_excludes "/opt/local"
    gfxs="sdl2 glut sdl1"
    if test -z "${optim_level_given}"; then
        optim_level="${optim_delivery}"
    fi
    mainbuilddir=
    cxxf=
    otherbuildflags=
    configureflags=
    delivery_check=yes
    unset bundle_addlibs; declare -a bundle_addlibs
    add_bundlelibs() { local _libpath=$1 _exepath=$2 _arg;shift 2;
        bundle_addlibs[${#bundle_addlibs[@]}]="${_libpath}"; bundle_addlibs[${#bundle_addlibs[@]}]="${_exepath}"
        while case "$1" in --*) true;; *) false;; esac; do bundle_addlibs[${#bundle_addlibs[@]}]="$1"; shift; done
        bundle_addlibs[${#bundle_addlibs[@]}]="$#";
        for _arg in "$@"; do bundle_addlibs[${#bundle_addlibs[@]}]="${_arg}"; done
    }
    case "${target_sysname}" in
        darwin*)
            bundle_basename="${bundle_basename}.app"
            other_bundle="${other_bundle}.app"
            bundledir="${deliverydir}/bundle/${bundle_basename}"
            bundle_bindir="${bundledir}/Contents/MacOS"
            bundle_resdir="${bundledir}/Contents/Resources"
            bundle_gtkresdir="${bundle_resdir}"
            bundle_toolsdir="${bundle_resdir}/bin"
            bundle_libpath="${bundle_resdir}/lib"; bundle_gtk_libpath="${bundle_libpath}/gtk"
            bundle_misc_libpath="${bundle_libpath}/misc"; bundle_tools_libpath="${bundle_libpath}"
            package_name="${deliverydir}/${bundle_pkgbasename}_macOS.dmg"

            get_libs() { otool -L "$@"; }
            if test "${build_sysmajor}" -lt 19; then
                cxxf="-arch x86_64 -arch i386"
            else
                cxxf="-arch x86_64 -arch arm64"
            fi
            find_macos_sdk "${cxxf}"
            chmod_args='-h'
            add_bundlelibs "${bundle_gtkresdir}/lib/gtk-2.0/2.10.0/engines" "${bundle_bindir}" "${GTK2_PREFIX}/lib/gtk-2.0/2.10.0/engines"/*.so
            ;;
        *bsd*|linux*)
            case " ${cxx_flags} " in 
                *" -m32 "*) cxxf="-m32 -D_GLIBCXX_USE_CXX11_ABI=0"; otherbuildflags="--cxx-std=98"
                            test -z "${optim_level_given}" && optim_level="-Os"
                            configureflags="-DBOOST_INTERNAL=1_35"; add_rpath_excludes "/usr/local/linux-gl32"
                            bundle_binsuffix=32; package_sysname=linux32;; 
                *) bundle_binsuffix=; package_sysname=linux64;;         
            esac
            bundledir="${deliverydir}/bundle/${bundle_basename}"
            bundle_bindir="${bundledir}/bin${bundle_binsuffix}"
            bundle_resdir="${bundledir}"
            test "$(basename "${bundle_bindir}")" = "bin" && bundle_gtkresdir="${bundle_resdir}" || bundle_gtkresdir="${bundle_bindir}"
            bundle_toolsdir="${bundle_resdir}/bin${bundle_binsuffix}"
            bundle_libpath="${bundle_resdir}/lib${bundle_binsuffix}"; bundle_gtk_libpath="${bundle_libpath}/gtk"
            bundle_misc_libpath="${bundle_libpath}/misc"; bundle_tools_libpath="${bundle_resdir}/lib${bundle_binsuffix}"
            package_name="${deliverydir}/${bundle_pkgbasename}_${package_sysname}.tar.bz2"

            get_libs() { ldd "$@" | sed -e 's/[[:space:]](0x[0-9a-fA-F]*)$//'; }
            case "${target_sysname}" in *bsd*) chmod_args='-h';; linux*) chmod_args='';; esac
            add_bundlelibs "${bundle_libpath}/alsa-lib" "${bundle_bindir}" "${VEGA_PREFIX}/lib/libasound.so"*
            add_bundlelibs "${bundle_libpath}/alsa-lib/plugins" "${bundle_bindir}" -R"${bundle_libpath}/alsa-lib" "-X/*/libpulse*.so*|libpulse*.so*" "${VEGA_PREFIX}/lib/alsa-lib"/*.so*
            export LD_LIBRARY_PATH="${VEGA_PREFIX}/lib:${VEGA_PREFIX}/lib/${target_arch}-linux-gnu:${GTK2_PREFIX}/lib:${GTK2_PREFIX}/lib/${target_arch}-linux-gnu${LD_LIBRARY_PATH:+:}${LD_LIBRARY_PATH}"
            ;;
        mingw*|msys*|cygwin*)
            case "${target_arch}" in 
                i[0-9]86|x86) otherbuildflags="--cc=${CC} --cxx-flags=-D_WIN32_WINNT=0x0501" # WINXP
                              bundle_binsuffix=32; package_sysname=windows32;;
                *) bundle_binsuffix=; package_sysname=windows64;;
            esac    
            bundledir="${deliverydir}/bundle/${bundle_basename}"
            bundle_bindir="${bundledir}/bin${bundle_binsuffix}"
            bundle_resdir="${bundledir}"
            test "$(basename "${bundle_bindir}")" = "bin" && bundle_gtkresdir="${bundle_resdir}" || bundle_gtkresdir="${bundle_bindir}"
            bundle_toolsdir="${bundle_resdir}/bin${bundle_binsuffix}"
            bundle_libpath="${bundle_bindir}"; bundle_gtk_libpath="${bundle_libpath}"
            bundle_misc_libpath="${bundle_libpath}"; bundle_tools_libpath="${bundle_libpath}"

            package_name="${deliverydir}/${bundle_pkgbasename}_${package_sysname}.zip"
            get_libs() { ldd "$@" | sed -e 's/[[:space:]](0x[0-9a-fA-F]*)$//'; }
            chmod_args='' #-c
            export PATH="${VEGA_PREFIX2:+${VEGA_PREFIX2}/${target_arch}-w64-mingw32/bin:${VEGA_PREFIX2}/bin:}${VEGA_PREFIX}/${target_arch}-w64-mingw32/bin:${VEGA_PREFIX}/bin:${GTK2_PREFIX}/${target_arch}-w64-mingw32/bin:${GTK2_PREFIX}/bin:${PATH}:/mingw${target_archbits}/bin:/mingw${target_archbits}/${target_arch}-w64-mingw32/bin"
            add_bundlelibs "${bundle_gtkresdir}/lib/gtk-2.0/2.10.0/engines" "${bundle_bindir}" "${GTK2_PREFIX}/lib/gtk-2.0/2.10.0/engines"/*.dll
            case "${target_arch}" in
                i[0-9]86|x86) 
                    export PATH="${PATH}:/c/windows/syswow64:/c/windows/system32"
                    add_bundlelibs "${bundle_libpath}" "${bundle_bindir}" --force "${target}/../deps/99_windows32-dlls"/*.dll
                    add_bundlelibs "${bundle_libpath}" "${bundle_bindir}" --force \
                        "${VEGA_PREFIX2}/bin"/lib{iconv-2,glib-2,gio-2,gobject-2,gmodule-2,gthread-2,harfbuzz-0,pango*-1,atk-1,gdk_pixbuf-2}*.dll;; 
                *) 
                    export PATH="${PATH}:/c/windows/system32"
                    add_bundlelibs "${bundle_libpath}" "${bundle_bindir}" "/c/WINDOWS/SYSTEM32/ucrtbase.dll" "/c/WINDOWS/SYSTEM32/downlevel/api-ms-win-crt-"*.dll;;
            esac
            ;;
        *)  chmod_args=''
            package_name="${deliverydir}/${bundle_pkgbasename}.zip"
            ;;
    esac

    test -z "${interactive}" || yesno "? Create Bundle ?" \
    && {
        # clean bundle dir
        echo "+ creating bundle ${bundledir}... "
        #rm -Rf "${bundledir}" "${deliverydir}/bundle" "${this_bundle}" "${other_bundle}" "${other_bundle_mountpoint}"
        rm -Rf "${deliverydir}/bundle"
        test -d "${deliverydir}/bundle" && { echo "!! error the bundle dir cannot be removed"; exit 1; }
        # copy bundle template
        mkdir -p "${bundledir}" || exit $?
        mkdir -p "${bundle_bindir}" || exit $?
        mkdir -p "${bundle_toolsdir}" || exit $?
        touch "${bundledir}"

        case "${target_sysname}" in
            darwin*)
                mkdir -p "${bundle_resdir}/share" || exit $?
                xcopy "${VEGA_PREFIX}/share/terminfo" "${bundle_resdir}/share" || exit $?
                xcopy "${bundle_src}"/ "${bundledir}" || exit $?
                # Applications link in bundle root
                rm -f "${deliverydir}/bundle/Applications"
                ln -s /Applications "${deliverydir}/bundle" || exit $?
                mkdir -p "${bundle_gtkresdir}/share/gtk-2.0/" && cp -pv "${GTK2_PREFIX}/share/themes/Quartz/gtk-2.0/gtkrc" "${bundle_gtkresdir}/share/gtk-2.0" || exit $?
                ;;
            linux*|*bsd*)
                mkdir -p "${bundle_resdir}/share" || exit $?
                xcopy "${VEGA_PREFIX}/share/terminfo" "${bundle_resdir}/share" || exit $?
                #xcopy "${mydir}/tools/linux_bundle/PrivateerGold/" "${bundledir}" || exit $?
                xcopy "${bundle_src}/Contents/MacOS/launcher.sh" "${bundle_bindir}" || exit $?
                mkdir -p "${bundledir}/etc" && cp -a "${VEGA_PREFIX}/share/alsa" "${bundledir}/etc/alsa" || exit $?
                ;;
            mingw*|msys*|cygwin*)
                #xcopy "${mydir}/tools/windows_bundle/PrivateerGold/" "${bundledir}" || exit $?
                mkdir -p "${bundle_gtkresdir}/share/gtk-2.0/" && cp -pv "${GTK2_PREFIX}/share/themes/MS-Windows/gtk-2.0/gtkrc" "${bundle_gtkresdir}/share/gtk-2.0" || exit $?
                ;;
        esac
    }
    if test -n "${force_color}"; then
        cxxf="${cxxf}${cxxf:+ }-fdiagnostics-color=always"
        export CLICOLOR_FORCE=1
    fi
    test -z "${interactive}" || yesno "? Build engines <$gfxs> with flags '$cxxf' ?" \
    && echo "+ building..." \
    && for gfx in ${gfxs}; do
        test "${build_tool}" = auto && build_tool=cmake
        builddir="${deliverydir}/${gfx}"
        test "${build_tool}" = "configure" && subdir=../ || subdir=
        test -z "${mainbuilddir}" && { mainbuilddir=${builddir}; mainsubdir=${subdir}; }

        "${mydir}/build.sh" --type=release --gfx=${gfx} --build="${builddir}" --optimize="${optim_level}" \
            --cxx-flags="${cxxf}" $otherbuildflags -- $configureflags || exit $?

        "${builddir}/tests/${subdir}test${exe}" || test -z "${delivery_check}" || exit $?

        cat "${priv_data}/New_Game" | "${builddir}/tools/${subdir}unicode-conv${exe}" -C \
            | "${builddir}/tools/${subdir}unicode-conv${exe}" -B - "${builddir}/New_Game.tmp" \
            && diff -ru "${priv_data}/New_Game" "${builddir}/New_Game.tmp" || test -z "${delivery_check}" || exit $?

        gfx=$(printf -- "${gfx}" | tr "[:lower:]" "[:upper:]")
        cp -pv "${builddir}/vegastrike${exe}" "${bundle_bindir}/vegastrike.${gfx}${exe}" || exit $?
    done || { mainbuilddir="${deliverydir}/sdl2"; test "${build_tool}" = "configure" && mainsubdir=../ || mainsubdir=; }

    # copy vssetup* & launcher from mainbuilddir to bundle
    for f in "${mainbuilddir}/setup/${mainsubdir}vssetup${exe}" "${mainbuilddir}/setup/${mainsubdir}vssetup_dlg${exe}" \
             "${mainbuilddir}/launcher/${mainsubdir}vslauncher${exe}" "${mainbuilddir}/launcher/${mainsubdir}vslauncher-gui${exe}"; do
        cp -pv "$f" "${bundle_bindir}"
    done

    # add the dynamic python libs to list of libs to be installed
    add_bundlelibs "${bundle_resdir}/lib${bundle_binsuffix}/pythonlibs" "${bundle_bindir}" --follow-link "${mainbuilddir}/lib${bundle_binsuffix}/pythonlibs"/*

    # add some libraries not referenced by ldd/otool (dlopen)
    add_bundlelibs "${bundle_libpath}" "${bundle_bindir}" $(find "${VEGA_PREFIX}/lib" \
        \( -iname '*.dylib' -or -iname '*.dylib.*' -or -iname '*.so' -or -iname '*.so.*' -or -iname '*.dll' \) \
        -and \( -iname '___NO___libpulse*' -or -iname 'libportaudio*' -or -iname 'libmysofa*' -or -iname 'libopus*' \) )

    test -z "${interactive}" || { yesno "? Finalize bundle (update libs, version, tools, ...) ?" || exit 1; }

    # Update Version in bundle
    git_rev2=$(get_git_rev); if test "${git_rev2}" != "${git_rev}"; then
        printf -- '\n!! ERROR Git revision ${git_rev} has changed to ${git_rev2} since build. Aborting...\n'; exit 1
    fi
    vega_version=$(sed -n -e 's/^[[:space:]]*#[[:space:]]*define[[:space:]][[:space:]]*VERSION[[:space:]][[:space:]]*"\([^"]*\).*/\1/p' \
                   ${mainbuilddir}/config.h)
    IFSbak=$IFS; IFS=$'\n'; for f in $(find "${deliverydir}/bundle" -iname '*.plist' 2>/dev/null); do
        awk 'BEGIN { ignore=1; }; /<key>(.*[vV]ersion.*|CFBundleGetInfoString)<\/key>/ { ignore=0; version="'${priv_version}'"; }; \
             />CFBundleGetInfoString<|>CFBundleVersion</ { if (!ignore) version="'${priv_version}' (Vegastrike '${vega_version}', git:'${git_rev}')"; } \
             />BuildVersion<|>SourceVersion</ { if (!ignore) version="'${vega_version}'"; } \
             /<string>/ { if (!ignore) { sub(/<string>[^<]*<\/string>/, "<string>" version "</string>"); ignore=1; }; }; \
             /CFBundleInfoDictionaryVersion/ { ignore=1;}; // { print $0; };' \
             "${f}" > "${f}.tmp" && mv "${f}.tmp" "${f}"
    done; IFS=$IFSbak

    # build checkModifierKeys/harshortcut tools and copy it to bundle bin dir
    case "${target_sysname}" in
        darwin*)
            test -n "${macos_sdk}" && checkmod_archs="-isysroot${macos_sdk} " || checkmod_archs=
            "${MAKE}" clean -C "${checkkeys_dir}" && "${MAKE}" -C "${checkkeys_dir}" OSX_VERSION_MIN="${target_osx_version_min}" ARCHS="${checkmod_archs}${cxxf}" || exit $?
            xcopy "${checkkeys_dir}/checkModifierKeys${exe}" "${bundle_bindir}" || exit $?
            ;;
        mingw*|cygwin*|msys*|win*|linux*|*bsd*)
            relbin=${bundle_bindir#${bundledir}/}
            archs=; shortcut_suffix=
            case " ${cxxf} " in *" -m32 "*) archs=-m32; shortcut_suffix=32;; esac
            case "${target_arch}" in i[0-9]86|x86) shortcut_suffix=32;; esac
            shortcuts="\{\\\"privgold\\\",\\\"${relbin}/vslauncher${exe}\\\",\\\"--game\\\",NULL,\\\"privsetup\\\",\\\"${relbin}/vslauncher${exe}\\\",\\\"--setup\\\",NULL,NULL\}"
            for bin_icon in "privgold:${priv_data}/priv-icon4.ico" "privsetup:${priv_data}/priv-setup.ico"; do
                "${MAKE}" distclean -C "${hardshortcut_dir}"  \
                && "${MAKE}" -C "${hardshortcut_dir}" CC="${CC}" ARCHS="${archs}" WINDRES="$(dirname "${CC}")/windres" \
                   SHORTCUT_CPPFLAGS="-I${mainbuilddir} -I${target} -DHAVE_VERSION_H -DSCM_PRODUCT_VERSION=\"\\\"${priv_version}\\\"\" -DSCM_PRODUCT_VERSION_RCNUM=$(echo "${priv_version}" | sed -e 's/\./,/g') -DVS_RESOURCE_ICON4_FILE=\"\\\"${bin_icon#*:}\\\"\"" \
                   SHORTCUT_CFLAGS="-DSHORTCUTS=${shortcuts}" || exit $?
                xcopy "${hardshortcut_dir}/hardshortcut${exe}" "${bundledir}/${bin_icon%%:*}${shortcut_suffix}${exe}" || exit $?
            done
            ;;
    esac

    # handle executables rpaths: vegastrike*
    "${rpath_tool}" "${rpath_excludes[@]}" -L${bundle_libpath} \
        "${bundle_bindir}"/vegastrike.*${exe} || exit $?

    # handle executables rpaths: vssetup_dlg
    "${rpath_tool}" "${rpath_excludes[@]}" -L${bundle_libpath} \
        "${bundle_bindir}/vssetup_dlg${exe}" || { printf -- '!! warning: vssetup_dlg (terminal) is not included in this delivery !!'; }

    # handle executables rpaths: vssetup(gtk)/vslauncher-gui(gtk)
    "${rpath_tool}" "${rpath_excludes[@]}" \
        -L${bundle_gtk_libpath} -R${bundle_libpath} \
        "${bundle_bindir}/vssetup${exe}" "${bundle_bindir}/vslauncher-gui${exe}" \
    || { printf -- '!! warning: vssetup/vslauncher-gui (gtk) are not included in this delivery !!'; }

    # handle executables rpaths: vslauncher
    "${rpath_tool}" "${rpath_excludes[@]}" \
        -L${bundle_libpath} \
        "${bundle_bindir}/vslauncher${exe}" \
    || { printf -- '!! error: vslauncher rpath !!'; exit 1; }

    # alplay OpenAL demo: useful to check if openal is fine, and important on linux to add dlopen lib dependencies
    alplay_bin=$(which -a "${VEGA_PREFIX2:+${VEGA_PREFIX2}/bin/alplay${exe}}" "${VEGA_PREFIX}/bin/alplay${exe}" 2> /dev/null | head -n1)
    if true && test -x "${alplay_bin}"; then
        mkdir -p "${bundle_toolsdir}" && cp -pv "${alplay_bin}" "${bundle_toolsdir}"
        "${rpath_tool}" "${rpath_excludes[@]}" \
            -L${bundle_libpath} -R${bundle_tools_libpath} "${bundle_toolsdir}/alplay${exe}" || exit $?
    fi

    # handle optional vegastrike tools executables rpaths: tools, objconv, vegaserver, test
    if true; then
        mkdir -p "${bundle_toolsdir}" && cp -pv "${mainbuilddir}"/vegaserver \
            "${mainbuilddir}/tools/${mainsubdir}"{vsrextract,vsrmake,unicode-conv}${exe} \
            "${mainbuilddir}/tests/${mainsubdir}"test${exe} \
            "${mainbuilddir}/objconv/${mainsubdir}"{base_maker,asteroidgen,mesh_tool,mesh_xml,replace,tempgen,trisort}${exe} "${bundle_toolsdir}"
        "${rpath_tool}" "${rpath_excludes[@]}" -L${bundle_tools_libpath} \
            "${bundle_toolsdir}"/{vegaserver,vsrextract,vsrmake,base_maker,asteroidgen,mesh_tool,mesh_xml,replace,tempgen,trisort,unicode-conv,test}${exe} || exit $?
    fi

    # optional ffplay ffmpeg demo and iconv: usefull to check ffmpeg is fine, and to check text encodings
    for name in ffplay iconv; do
        bin=$(which -a "${VEGA_PREFIX2:+${VEGA_PREFIX2}/bin/${name}${exe}}" "${VEGA_PREFIX}/bin/${name}${exe}" 2> /dev/null | head -n1)
        if true && test -x "${bin}"; then
            mkdir -p "${bundle_toolsdir}" && cp -pv "${bin}" "${bundle_toolsdir}"
            "${rpath_tool}" "${rpath_excludes[@]}" \
                -L${bundle_misc_libpath} -R${bundle_tools_libpath} "${bundle_toolsdir}/${name}${exe}" || exit $?
        fi
    done

    # optional gtk-demo files: usefull to check gtk dist is fine
    if true && test -x "${GTK2_PREFIX}/bin/gtk-demo${exe}"; then
        mkdir -p "${bundle_toolsdir}" && cp -pv "${GTK2_PREFIX}/bin/gtk-demo${exe}" "${bundle_toolsdir}"
        mkdir -p "${bundle_gtkresdir}/share/gtk-2.0/demo" \
        && xcopy "${GTK2_PREFIX}/share/gtk-2.0/demo"/* "${bundle_gtkresdir}/share/gtk-2.0/demo"
        "${rpath_tool}" "${rpath_excludes[@]}" \
            -L${bundle_gtk_libpath} -R${bundle_tools_libpath} \
            "${bundle_toolsdir}/gtk-demo${exe}" || exit $?
    fi

    # gdk-pixbuf dynamic loaders when gdk not built with builtin_loaders
    if true && gdk_loaders="gdk-pixbuf-2.0/2.10.0/loaders" \
    && test -d "${GTK2_PREFIX}/lib/${gdk_loaders}"; then
        mkdir -p "${bundle_toolsdir}" && cp -pv "${GTK2_PREFIX}/bin/gtk-demo${exe}" "${bundle_toolsdir}" \
        && mkdir -p "${bundle_gtkresdir}/lib/${gdk_loaders}" \
        && xcopy "${GTK2_PREFIX}/lib/${gdk_loaders}"* "${bundle_gtkresdir}/lib/${gdk_loaders}/.." --exclude "*.dll.a" --exclude '*.a' \
        && "${rpath_tool}" "${rpath_excludes[@]}" \
            -L${bundle_gtk_libpath} -R${bundle_tools_libpath} -R${bundle_libpath} \
            "${bundle_toolsdir}/gtk-demo${exe}" "${bundle_gtkresdir}/lib/${gdk_loaders}"/* || exit $?
    fi

    # Add manually some LIBs
    # They are stored in bundle_addlibs array with format:
    #   <destdir> <execpath> [<--options>] <NBFILES> [file1 [file2 [...]]] [<destdir2> ...]
    _i=0; while test ${_i} -lt ${#bundle_addlibs[@]}; do
        _cp_opt="-a"; _overwrite=; _dst="${bundle_addlibs[${_i}]}"; _exec_path="${bundle_addlibs[$((_i+1))]}"; _i=$((_i+2))
        while case "${bundle_addlibs[${_i}]}" in --*) true;; *) false;; esac; do
            case "${bundle_addlibs[${_i}]}" in 
                --follow-link) _cp_opt="-L";; 
                --force) _overwrite=yes;;
            esac; _i=$((_i+1))
        done
        _count="${bundle_addlibs[${_i}]}"
        _i=$((_i+1)); _start=${_i}
        unset _libs; declare -a _libs; while test ${_i} -lt $((_start+_count)); do
            _lib="${bundle_addlibs[${_i}]}"; _i=$((_i+1))
            case "${_lib}" in
                -[REX]?*) _dstlib=${_lib};; # allow to pass certain options to rpath.sh
                *)
                    _dstlib="${_dst}/$(basename "${_lib}")";
                    test -f "${_dstlib}" -a -z "${_overwrite}" && continue
                    mkdir -p "${_dst}" && cp -pv "${_cp_opt}" "${_lib}" "${_dst}" || exit $?
                    test -L "${_dstlib}" && continue;;
            esac
            _libs[${#_libs[@]}]="${_dstlib}"
        done
        test ${#_libs[@]} -le 0 || "${rpath_tool}" "${rpath_excludes[@]}" -E"${_exec_path}" -L"${bundle_libpath}" \
            -R"${bundle_gtk_libpath}" -R"${bundle_misc_libpath}" "${_libs[@]}" || exit $?
    done

    #fix perms
    find "${deliverydir}/bundle" \! -perm '+u=w' -print0 | xargs -0 chmod ${chmod_args} 'u+w'

    # package pref / suff
    case "${package_name}" in *.tar.*) package_pref=${package_name%.tar.*};; *) package_pref=${package_name%.*};; esac
    package_ext=${package_name#${package_pref}}

    # Merge the generated bundle with the ARM64 one if present
    other_bundle_ref_archive="${other_bundle_ref_dir}/$(basename "${package_name%${package_ext}}")-MoreArchs${package_ext}"
    if ! test -f "${other_bundle_ref_archive}" && test "${do_delivery}" = "dev"; then
        other_bundle_ref_archive=$(ls -d -t "${other_bundle_ref_dir}/${package_basename%-*}"*"-MoreArchs${package_ext}" 2>/dev/null | head -n1)
    fi
    if test -f "${other_bundle_ref_archive}"; then
        other_bundle_mountpoint="$(dirname "${other_bundle}")"
        mkdir -p "${other_bundle_mountpoint}"
        trap_backup=$(trap 2>&1 | awk '/EXIT$/ { print $3 }')
        archive_cleanup() { true; }
        trap archive_cleanup EXIT
        eval trap_backup=${trap_backup:-"-"}
        printf -- "+ extracting ${other_bundle_ref_archive}...\n\n"
        case "${package_ext}" in
            .[dD][mM][gG])
                archive_cleanup() { hdiutil unmount -quiet "${other_bundle_mountpoint}" && echo "+ ${other_bundle_mountpoint} unmounted"; }
                hdiutil mount -quiet -readonly -mountpoint "${other_bundle_mountpoint}" "${other_bundle_ref_archive}";;
            .tar.xz) (cd "${other_bundle_mountpoint}" && "${tar}" xJf "${other_bundle_ref_archive}");;
            *) false;;
        esac; test $? -eq 0 || { echo "ERROR, cannot extract ${other_bundle_ref_archive}"; exit 1; }
        if ! test -d "${other_bundle}" && test "${do_delivery}" = "dev"; then
            other_bundle=$(ls -d -t "$(dirname "${other_bundle}")/${bundle_basename%%-*}"* 2>/dev/null | head -n1)
        fi
        printf -- "+\n+ Merging architectures of ${bundle_basename} and ${other_bundle#${deliverydir}}...\n+\n"
        test -d "${other_bundle}" || exit 1
        rm -f "${other_bundle}/Contents/Resources/data"
        mv -v "${bundledir}" "${this_bundle}" || exit $?
        "${mydir}/deps/mergelibs.sh" -S -N -U -X"${bundle_resdir#${bundledir}}/data" "${this_bundle}" "${other_bundle}" "${bundledir}" || exit $?
        archive_cleanup; trap "${trap_backup}" EXIT
        rm -Rf "${other_bundle}" "${this_bundle}" "${other_bundle_mountpoint}"
        echo
    elif case "${target_sysname}" in darwin*) true;; *) false;; esac; then
        printf -- "\n!!\n!! WARNING: macOS additional architectures package ${other_bundle_ref_archive} not found\n!!\n\n"
    fi

    # Display list of libs
    case "${target_sysname}" in
        linux*|*bsd*) export LD_LIBRARY_PATH="${bundle_libpath}:${bundle_gtk_libpath}:${bundle_misc_libpath}:${bundle_tools_libpath}";;
    esac
    echo "+ Libraries:"
    for f in $(find "${bundle_bindir}" "${bundle_libpath}" -type f); do
        test -f "$f" && get_libs "$f" 2> /dev/null | grep -Ev '^[^[:space:]]'
    done | sort | uniq
    case "${target_sysname}" in
        darwin*) for f in "${bundle_bindir}"/{vegastrike*${exe},vssetup*${exe},vslauncher*${exe}}; do lipo -info "$f" 2> /dev/null | sed -e 's|.*/||'; done;;
    esac

    # Put data in bundle
    if test "${do_delivery}" != "nodata"; then
        test -z "${interactive}" || yesno "? Copy whole data in bundle (${priv_data}) ?" \
        && { echo "+ copying data..."; rm -f "${bundle_resdir}/data" || exit 1; \
             xcopy --exclude "/bin" --exclude "/tofix" \
                   --exclude "/generated_factions.xml" --exclude "/0.html" \
                   --exclude "*.py.orig" --exclude "*.py.rej" --exclude "*.txt.orig" --exclude "*.txt.rej" \
                   "${priv_data}/" "${bundle_resdir}/data"; } || exit $?
    fi

    # fix permissions
    find "${deliverydir}/bundle" -perm '+o=w' -print0 | xargs -0 chmod ${chmod_args} 'g-w,o-w'
    find "${deliverydir}/bundle" -perm '+g=w' -print0 | xargs -0 chmod ${chmod_args} 'g-w,o-w'
    find "${deliverydir}/bundle" -perm '+u=x' -print0 | xargs -0 chmod ${chmod_args} 'g+x,o+x'
    find "${deliverydir}/bundle" -perm '+u=r' -print0 | xargs -0 chmod ${chmod_args} 'g+r,o+r'

    # Create Package
    test -z "${interactive}" || yesno "? Create Package (${package_name}) ?" || exit 1
    echo "+ creating Package (${package_name})"
    rm -f "${package_name}" || exit $?
    case "${package_name}" in
        *.[Dd][Mm][Gg])
            # Create DMG
            #formats UDBZ(bz2,10.4) UDCO ULFO(lzfe,10.11) # -fs HFS+ # -type UDIF|SPARSE|SPARSEBUNDLE
            hdiutil create -fs "HFS+" -format "UDBZ" -volname "${bundle_pkgbasename}" \
                -srcfolder "$(dirname "${bundledir}")" "${package_name}" || exit $?
            ;;
        *.[bB][zZ]2)
            (cd "$(dirname "${bundledir}")" && "${tar}" cjf "${package_name}" "$(basename "${bundledir}")" ) || exit 1
            ;;
        *.[zZ][iI][pP])
            (cd "$(dirname "${bundledir}")" && ${zip} -q -r "${package_name}" "$(basename "${bundledir}")" ) || exit 1
            ;;
    esac
    (cd "$(dirname "${package_name}")" && shasum -a256 "$(basename "${package_name}")" > "${package_name}.sha256" \
     && du -sh "${package_name}" | awk '{ print $1 }' | tr '\n' ' ' && cat "${package_name}.sha256")

    if test "${do_delivery}" = "nodata"; then
        echo "+ creating data link"
        rm -f "${bundle_resdir}/data" || exit 1
        ln -sv "${priv_data}" "${bundle_resdir}/data" || exit $?
    fi

    popd >/dev/null
}

do_delivery_fun() {
    local deliverydir="${mydir}/${buildpool}/delivery"
    case "${target_sysname}" in 
        linux*) case " ${cxx_flags} " in *" -m32 "*) deliverydir="${deliverydir}32";; esac;; 
        mingw*|cygwin*) case "${target_arch} " in i[0-9]86*|x86) deliverydir="${deliverydir}32";; esac;; 
    esac

    echo "*** creating delivery dir '${deliverydir}'..."

    mkdir -p "${deliverydir}" || exit $?
    deliverydir=$(cd "${deliverydir}" && pwd || exit 1)
    echo
    do_delivery_fun_real "${deliverydir}" "$(test -t && echo "yes")" 2>&1 | tee "${deliverydir}/build.log"
}

do_generate_python_cpp_api() {
    local _cpp_api_dir=$1 _python=$2 _force_color=$3; shift; shift; shift
    test -x "${_python}" || _python=$(which -a python2.7 python2 python 2>/dev/null | head -n1)
    test -z "${_cpp_api_dir}" && _cpp_api_dir="${builddir}/Python_CppApi_Mock"
    mkdir -p "${_cpp_api_dir}"; test -d "${_cpp_api_dir}" || return 1;
    rm -f ${_cpp_api_dir}/*.py
    printf -- '_cache=dict()\n' >> ${_cpp_api_dir}/VS.py
    gawk_bin=$(which -a gawk awk 2>/dev/null | head -n1)
    ${gawk_bin} 'BEGIN{ debug(1, "BEGIN"); cur_line=0; cur_file=""; module=""; outclass=""; class=""; global=0; API_DIR="'${_cpp_api_dir}'"; } \
        function macro_args(str) { sub(/^[^(]*\([[:space:]]*/, "", str); sub(/\)[;[:space:]]*$/, "", str); return str; }; \
        function args_split(args, array) { \
            n=patsplit(args, array, /[[:space:]]*[[:alnum:]_]*\([^)]*\)|[[:space:]]*[^,()]+/); \
            for (i=1;i<=n;i++)  { sub(/^[[:space:]]*/, "", array[i]); sub(/[[:space:]]*$/, "", array[i]); }; return n; }\
        function macro_args_split(str, array) { args=macro_args(str); return args_split(args, array); };                \
        function unquote(str) { return gensub(/^[[:space:]]*"([^"]*)"[[:space:]]*$/, "\\1", 1, str); };                 \
        function debug(lvl, str) { n=patsplit(FILENAME, array, /[^/\\]*/); print array[n] ":" cur_line "; " str > "/dev/stderr"; };  \
        function python_math(fun,indent) { return indent "try:\n" indent "    import math; return math." fun "(a1)\n" indent "except:\n" indent "    pass\n" \
                                                  indent "global _cache\n" indent "if \"" fun "\"+\".\"+str(a1) in _cache:\n" \
                                                  indent "    return _cache[\"" fun "\"+\".\"+str(a1)]\n" \
                                                  indent "import os; inout=os.popen2(\"'${_python}' -B -E -c \\\"import math; print math." fun "(\"+str(a1)+\")\\\"\")\n" \
                                                  indent "ret=float(inout[1].read()); _cache[\"" fun "\"+\".\"+str(a1)]=ret; return ret" }; \
        function write_export(module, class, method, ret, line) { ;                                     \
            indent = class == "" ? "" : "    "; \
            outfile = API_DIR "/" module ".py"; \
            n=patsplit(cur_file, array, /[^/\\]*/); srcfile=array[n]; \
            if (class != outclass) print "" >> outfile; \
            if (class != "" && class != outclass) { \
                print "# Generated from " srcfile ":" cur_line ": " line >> outfile; \
                if (class in classes) { print "class " class "(" class "):" >> outfile; } \
                else { print "class " class ":" >> outfile; classes[class] = 1; }; }; \
            if (class != outclass) outclass=class; \
            print indent "# Generated from " srcfile ":" cur_line ": " line >> outfile; \
            print indent "def " method "(a1=None, a2=None, a3=None, a4=None, a5=None, a6=None, a7=None, a8=None, " \
                                        "a9=None, a10=None, a11=None, a12=None, a13=None, a14=None, a15=None):" >> outfile; \
            doreturn=indent "    return"; \
            if (module "." class "::" method == "VS.::sqrt") { doreturn=python_math("sqrt",indent "    "); ret=""; } \
            else if (module "." class "::" method == "VS.::exp") { doreturn=python_math("exp",indent "    "); ret=""; } \
            else if (module "." class "::" method == "VS.::log") { doreturn=python_math("log",indent "    "); ret=""; } \
            else if (module "." class "::" method == "VS.un_iter::current") { ret="Unit()"; } \
            else if (module "." class "::" method == "VS.::vsConfig") { ret="\"'"${_force_color}"'\" if str(a1+\"/\"+a2) == \"log/colorize\" else \"\""; } \
            else if (module "." class "::" method == "Base.::GetRandomBarMessage") { ret="(\"\",\"\")"; } \
            else if (module "." class "::" method == "VS.Unit::getSubUnits") { ret="un_iter()"; } \
            else if (module "." class "::" method == "VS.::LogPrint") { doreturn=indent "    " "import sys; sys.stdout.write(a1); return"; ret=1; } \
            print doreturn " " ret >> outfile; \
        }; \
        function fixret(ret) { \
            sub(/^[[:space:]]*/, "", ret); sub(/\/\/.*/, "", ret); sub(/\/\*.*/, "", ret); sub(/[[:space:]]*$/, "", ret); \
            ret = gensub(/(^|[,[:space:]]|)([-+]?[0-9]*\.?[0-9]+)([lL]?[fF])([,[:space:]]|$|)/, "\\1\\2\\4", "g", ret); \
            ret = gensub(/(^|[,[:space:]]|)([-+]?[0-9]+\.?[0-9]*)([lL]?[fF])([,[:space:]]|$|)/, "\\1\\2\\4", "g", ret); \
            if (ret ~ /^Q?Vector[[:space:]]*\(/) ret=gensub(/[^(]*\(([^)]*)\)[[:space:]]*$/, "[ \\1 ]", 1, ret); \
            else if (ret == "std::string()") ret = "\"\"";                                              \
            else if (ret ~ /std::string[[:space:]]*\(/) ret = gensub(/std::string[[:space:]]*\((.*)\)$/, "\\1", 1, ret); \
            else if (ret == "true") ret = "True"; else if (ret == "false") ret = "False";               \
            else if (ret == "bool()") ret = "False"; else if (ret == "int()") ret = "0";                \
            else if (ret == "float()") ret = "0.0"; else if (ret == "UnitWrapper()") ret = "Unit()";    \
            return ret; };                                                                              \
        // { if (cur_file != FILENAME) { cur_file=FILENAME; cur_line=0; }; cur_line++; }                \
        /^[[:space:]]*PYTHON_(BASE_|)BEGIN_(INHERIT_|)CLASS[[:space:]]*\(/ {                            \
            n = macro_args_split($0, array);                                                            \
            module=array[1]; class=unquote(array[$0 ~ /.*INHERIT_CLASS.*/ ? 4 : 3]);                    \
            debug(1, "CLASS " module "." class); };                                                     \
        /^[[:space:]]*PYTHON_END_CLASS[[:space:]]*\(/ { class=""; };                                    \
        /^[[:space:]]*PYTHON_BEGIN_MODULE[[:space:]]*\(/ {                                              \
            n = macro_args_split($0, array); module=array[1];                                           \
            debug(1, "MODULE:" module); };                                                              \
        /^[[:space:]]*PYTHON_END_MODULE[[:space:]]*\(/ { module=""; };                                  \
        /^[[:space:]]*PYTHON_DEFINE_GLOBAL[[:space:]]*\(/ {                                             \
            n = macro_args_split($0, array); mymodule=array[1]; mymethod=unquote(array[3]); myret="0";  \
            debug(1, "FUNC: " mymodule "." mymethod " = " myret);                                       \
            write_export(mymodule, "", mymethod, myret, $0); };                                         \
        /^[[:space:]]*PYTHON_DEFINE_METHOD(_DEFAULT|)[[:space:]]*\(/ {                                  \
            n = macro_args_split($0, array); mymethod=unquote(array[3]); myret="0";                     \
            debug(1, "METHOD D: " module "." class "::" mymethod " = " myret);                          \
            write_export(module, class, mymethod, myret, $0); };                                        \
        /^[[:space:]]*voidWRAPPED[0-9][[:space:]]*\(/ {                                                 \
            if (module == "") module="VS"; if (class == "") class="Unit";                               \
            n = macro_args_split($0, array); mymethod=unquote(array[1]); myret="0";                     \
            debug(1, "METHOD vW: " module "." class "::" mymethod " = " myret);                         \
            write_export(module, class, mymethod, myret, $0); };                                        \
        /^[[:space:]]*WRAPPED[0-9][[:space:]]*\(/ {                                                     \
            if (module == "") module="VS"; if (class == "") class="Unit";                               \
            n = macro_args_split($0, array); mymethod=unquote(array[2]); myret=fixret(array[n]);        \
            debug(1, "METHOD W: " module "." class "::" mymethod " = " myret);                          \
            write_export(module, class, mymethod, myret, $0); };                                        \
        /^[[:space:]]*voidEXPORT_[[:alpha:]]*[[:space:]]*\(/ {                                          \
            if (module == "") module="VS";                                                              \
            n = macro_args_split($0, array); mymethod=unquote(array[1]); myret="0";                     \
            debug(1, "FUNC vE: " module "." mymethod " = " myret);                                      \
            write_export(module, "", mymethod, myret, $0); };                                           \
        /^[[:space:]]*EXPORT_[[:alpha:]]*[[:space:]]*\(/ {                                              \
            if (module == "") module="VS"; if (class == "") class="Unit";                               \
                n = macro_args_split($0, array); mymethod=unquote(array[1]); myret=fixret(array[2]);    \
            debug(1, "FUNC E: " module "." mymethod " = " myret);                                       \
            write_export(module, "", mymethod, myret, $0); };                                           \
        /^[[:space:]]*voidEXPORT_[[:alpha:]][0-9]*[[:space:]]*\(/ {                                     \
            if (module == "") module="VS"; if (class == "") class="Unit";                               \
            n = macro_args_split($0, array); mymethod=unquote(array[1]); myret="0";                     \
            debug(1, "METHOD vE: " module "." class "::" mymethod " = " myret);                         \
            write_export(module, class, mymethod, myret, $0); };                                        \
        /^[[:space:]]*EXPORT_[[:alpha:]]*[0-9][[:space:]]*\(/ {                                         \
            if (module == "") module="VS"; if (class == "") class="Unit";                               \
            n = macro_args_split($0, array); mymethod=unquote(array[2]); myret=fixret(array[1] "()");   \
            debug(1, "METHOD E: " module "." class "::" mymethod " = " myret);                          \
            write_export(module, class, mymethod, myret, $0); };                                        \
        ' "$@"
}
do_checkpython_fun() { (
    # CHECK PYTHON IN PROGRESS
    # Need VS builtins: BASE, VS, ...
    local do_checkpython=$1
    local force_color=$2
    local pythonlibs_dir=${3:-"${builddir}/${target_libdirname}/pythonlibs"}
    local pymods="modules/builtin modules/builtin/encodings quests missions modules/ai modules bases"
    local python_prefix=$(python2.7-config --prefix)
    local _m _p err _pyargs _check_filter _compile
    export PYTHONPATH= PYTHONHOME=
    for _m in ${pymods}; do
        _m="${priv_data}/$_m"
        echo $_m
        PYTHONPATH="${PYTHONPATH}${PYTHONPATH:+:}$_m"
        PYTHONHOME="${PYTHONHOME}${PYTHONHOME:+:}$_m"
    done
    PYTHONPATH="${PYTHONPATH}${PYTHONPATH:+:}${pythonlibs_dir}"
    local _cpp_api_dir="${builddir}/Python_CppApi_Mock"
    #do_generate_python_cpp_api ${_cpp_api_dir}
    do_generate_python_cpp_api ${_cpp_api_dir} ${PYTHON} "${force_color}" \
        "${target}/src/python/unit_wrapper.cpp" \
        "${target}/src/python/briefing_wrapper.cpp" \
        "${target}/src/cmd/base_init.cpp" \
        "${target}/src/cmd/script/director_generic.cpp" \
        "${target}/src/python/python_unit_wrap.h" \
        "${target}/src/python/star_system_exports.h"
    export PYTHONHOME="${_cpp_api_dir}:$PYTHONHOME" #:${python_prefix}"
    export PYTHONPATH="${_cpp_api_dir}:$PYTHONPATH"
    echo "PYTHONHOME  ${PYTHONHOME}"
    echo "PYTHONPATH  ${PYTHONPATH}"
    echo "DATADIR     ${priv_data}"
    pythoncmd=
    local -a errors
    local -a warnings
    local check_filter
    case "${do_checkpython}" in
        "bytecode"|"bytecode:"*) _pyargs=""; _check_filter=${do_checkpython#bytecode:}; _compile=yes;;
        *) _pyargs="-B"; _check_filter=${do_checkpython};;
    esac
    for _m in $pymods; do
        for _p in ${priv_data}/${_m}/*.py; do
            # the redirection stuff with fd 3 inverts stderr and stdout in order to put stderr in variable while displaying stdout
            if test -r "${_p}" -a \( "${_check_filter}" = "yes" -o "${_check_filter}" = "bytecode" -o "${_check_filter}" = "$(basename "$_p")" \); then
                # python compile/run check
                # PYTHONPATH="$(dirname "${_p}"):${PYTHONPATH}"
                false && case "${_m}" in "modules/builtin"|"modules/builtin"/*) true;; *) false;; esac \
                || { cd "${priv_data}" \
                     && err=$( { { ${PYTHON} ${_pyargs} -c "import $(basename "${_p%.py}")"; } 2>&1 >&3; } 3>&2 ) \
                     && { test -z "${_compile}" || err=$( { { ${PYTHON} ${_pyargs} -O -c "import $(basename "${_p%.py}")"; } 2>&1 >&3; } 3>&2 ); } \
                     && printf -- "${err}+ OK : $_p\n" \
                     || { printf -- "${err}\n******* ERROR $_p\n"; \
                         case "${_m}" in "modules/builtin"|"modules/builtin"/*) true;; *) false;; esac \
                             && warnings[${#warnings[@]}]="$_p"$'\n'"${err}"$'\n' \
                             || errors[${#errors[@]}]="$_p"$'\n'"${err}"$'\n'; }
                }
                # PYC check
                if test -r "${_p}c"; then test "${_p}" -nt "${_p}c" && warnings[${#warnings[@]}]="${_p}: PYC older than PY";
                else warnings[${#warnings[@]}]="${_p}: no PYC"; fi
                test $(echo "${py_version}" | awk -F'.' '{ print $1*1000000 + $2*1000 }') -lt $((3*1000000+5*1000)) \
                && if test -r "${_p}o"; then test "${_p}" -nt "${_p}o" && warnings[${#warnings[@]}]="${_p}: PYO older than PY";
                else warnings[${#warnings[@]}]="${_p}: no PYO"; fi

                # spaces/tabs check
                gawk_bin=$(which -a gawk awk 2>/dev/null | head -n1)
                err=$("${gawk_bin}" \
                'BEGIN{ debuglvl=0; instr=0; nstack=0; lines=""; spclines=""; tablines="";          \
                        cloze["\""]="\""; cloze["\"\"\""]="\"\"\""; cloze["("]=")"; cloze["["]="]"; \
                        cloze["'"'"'"]="'"'"'"; cloze["'"'''"'"]="'"'''"'"; cloze["{"]="}"; };      \
                 function debug(lvl,message) { if (debuglvl>=lvl) print message >> "/dev/stderr"; };\
                 // { comment=0; oldnstack=nstack; }; /^[[:space:]]*#/ { comment=1; }                    \
                 // { if (!comment) for (i=1; i <= length($0); ++i) {                               \
                        if (substr($0,i,1)=="\\") { ++i; debug(2, FNR "," i ": escape " substr($0,i,1)); } \
                        else if (instr == 0 && substr($0,i,1) == "#") break ;                       \
                        else if (nstack > 0 && substr($0,i,length(stack[nstack-1])) == stack[nstack-1]) { \
                            if (instr==0 || substr($0,i,1) ~ /['"'"'"]/) {                          \
                                debug(2, FNR "," i ": close " stack[nstack-1]); instr=0; i+=length(stack[nstack-1])-1; --nstack; }; } \
                        else if (instr==0) for (n=3; n>0; n--) { if (substr($0,i,n) in cloze) {     \
                            stack[nstack++]=cloze[substr($0,i,n)]; if (substr($0,i,1) ~ /["'"'"']/) instr=1; \
                               debug(2, FNR "," i ": open " instr " " substr($0,i,n)); i+=n-1; break; }; }; }; }; \
                 /^\t\t*[ ]|^[ ][ ]*\t/ { if (oldnstack == 0) { debug(1, FNR ": spaces/tabs in the same line"); lines = lines " " FNR }; }; \
                 /^\t/ { if (oldnstack==0) tablines=tablines " " FNR; }; /^ / { if (oldnstack==0) spclines=spclines " " FNR; } \
                 END { if (lines != "") print FILENAME ": mixed tab/spaces in the same line: " lines; if (nstack != 0) { \
                           print FILENAME ": ERROR: the stack is not empty: " nstack; for (i=0; i < nstack; ++i) print "  " stack[i] >> "/dev/stderr"; }; \
                       if (length(spclines) < length(tablines)) mlines=spclines; else mlines=tablines; \
                       if (spclines != "" && tablines != "") print FILENAME ": mixed spaces/tab in file: " mlines;  }; ' \
                "${_p}")
                IFSbak=$IFS; IFS=$'\n'; for e in ${err}; do
                     warnings[${#warnings[@]}]="$e" && printf -- "warning: %s\n" "$e"
                 done; IFS=$IFSbak
            fi
        done
    done
    if test ${#warnings[@]} -gt 0; then
        printf -- "\n${#warnings[@]} WARNING(S):\n"
        printf -- '!  %s\n' "${warnings[@]}"
    fi
    printf -- "\n${#errors[@]} ERROR(S):\n"
    if test ${#errors[@]} -gt 0; then
        printf -- '!! %s\n' "${errors[@]}"
    fi
    ### PRINT Modules importing modules:
    ###for f in modules; do printf -- "** $f:"; grep -E ".*(import|from).*${f%.py}" -Ir data | awk -F ':' '{ print $1 }' | sort | uniq | tr '\n' ' '; printf -- '\n'; done
    return ${#errors[@]}
); }

### DO DIST TARBALL
if test -n "${do_dist}"; then
    do_dist_fun "${do_dist}"
    exit $?
fi
### DO DELIVERY
if test -n "${do_delivery}"; then
    do_delivery_fun
    exit $?
fi
### DO_CHECKPYTHON
if test -n "${do_checkpython}"; then
    do_checkpython_fun "${do_checkpython}"
    exit $?
fi

pushd "${target}" >/dev/null || exit 1
### BUILD
do_build_fun; ret=$?

### RUN
if test $ret -eq 0; then
    do_run_fun; ret=$?
fi

popd >/dev/null # build
popd >/dev/null # target

exit $ret

# this can be added to cmake args if you don't want to use FindGTK2.cmake
#-DGTK2_LIBRARIES="$(pkg-config --libs gtk+-2.0)" -DGTK2_INCLUDE_DIRS=" " -DGTK2_DEFINITIONS="$(pkg-config --cflags gtk+-2.0)" \

#-D_GNU_SOURCE=1 \

#-DJPEG_FOUND=1 -DJPEG_INCLUDE_DIR="/usr/local/specific/jpeg9c/include" \
#    -DJPEG_LIBRARIES="/usr/local/specific/jpeg9c/lib/libjpeg.dylib" \

#-DHAVE_BOOLEAN=1
#-DFFMPEG_FOUND=1 \
#    -DFFMPEG_LIBRARIES="/opt/local/lib/libavcodec.dylib /opt/local/lib/libswscale.dylib /opt/local/lib/libavcodec_avformat.dylib /opt/local/lib/libavformat_avio.dylib" \
#    -DFFMPEG_INCLUDE_DIRS="/opt/local/lib" \
#    -DHAVE_LIBSWSCALE_SWSCALE_H=0 -DHAVE_LIBSWSCALE_SWSCALE_H=1 -DHAVE_LIBAVCODEC_AVCODEC_H=1 -DHAVE_LIBAVFORMAT_AVFORMAT_H=1 -DHAVE_LIBAVFORMAT_AVIO_H=1 \


#    -DOPENAL_LIBRARY="${macos_sdk}/System/Library/Frameworks/AudioToolbox.framework/AudioToolbox.tbd/Headers" \
#    -DOPENAL_INCLUDE_DIR="${macos_sdk}/System/Library/Frameworks/AudioToolbox.framework/AudioToolbox.tbd" \

#    -DGTK2_GTK_LIBRARY="-L/opt/local/lib -lgtk-quartz-2.0 -lgdk-quartz-2.0 -framework Cocoa -lpangocairo-1.0 -lpango-1.0 -latk-1.0 -lcairo -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0 -lintl -Wl,-framework -Wl,CoreFoundatio" GTK2_GTK_INCLUDE_DIR="/opt/local/include/gtk-2.0" \

