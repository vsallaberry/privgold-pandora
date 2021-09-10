#!/bin/bash
mydir=$(cd "$(dirname "$0")"; pwd)
mydirname=$(basename "${mydir}")

if false; then
    priv_data="${mydir}/../privgold/privgold/trunk/data"
    target="${mydir}/../privgold/privgold/trunk/engine"
else
    priv_data="${mydir}/data"
    target="${mydir}/vegastrike"
fi

builddir=build

unset CFLAGS CXXFLAGS LDFLAGS CPPFLAGS OBJCFLAGS OBJCPPFLAGS
cc_flags=""
gcc_cc_flags=""
clang_cc_flags=""
#cxx_flags="-fpermissive -Wno-deprecated-declarations -Wno-unused-local-typedefs -Wno-strict-aliasing"
cxx_flags="-Wno-deprecated-declarations -Wno-unused-local-typedefs -Wno-strict-aliasing"
#-std=c++03
gcc_cxx_flags=""
clang_cxx_flags="-stdlib=libc++"
#compiler=/usr/local/gcc/gcc-6.5.0_ada/bin/gcc
compiler=clang
make_jobs=$(sysctl hw.ncpu | awk '{ print $2 }')
test -n "${make_jobs}" && make_jobs=$((make_jobs+1)) || make_jobs=2

#compiler=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang
export MAKE=$(which -a gmake make | head -n 1)
export PYTHON="/usr/bin/python2.7"
export PYTHONPATH="/usr/lib/python2.7"
export PYTHONHOME="${PYTHONPATH}"

#If you'd like to specify the installation of Python to use, you should
#modify the following cache variables:
#- path to the python library
export PYTHON_LIBRARY=/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/config/libpython2.7.dylib
#- path to where Python.h is found
export PYTHON_INCLUDE_DIR=/System/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7

export GTK2_HOME=/opt/local/include/gtk-2.0

#export PKG_CONFIG_PATH=/usr/local/libpng12/lib/pkgconfig:/usr/lib/pkgconfig:/opt/X11/lib/pkgconfig:/opt/local/lib/pkgconfig
PKG_CONFIG_PATH=/usr/local/specific/libpng12/lib/pkgconfig
PKG_CONFIG_PATH+=:/usr/local/specific/ffmpeg1/lib/pkgconfig
PKG_CONFIG_PATH+=:/usr/local/specific/libsdl1/lib/pkgconfig
PKG_CONFIG_PATH+=:/usr/lib/pkgconfig
PKG_CONFIG_PATH+=:/opt/local/lib/pkgconfig
export PKG_CONFIG_PATH

build_preferred="cmake configure"
build_tool=auto
build_type=RelWithDebInfo

do_run=
do_debug=
do_setup=
do_dist=
interactive=
force_clean=

show_help() {
    local _ret=$1
    echo "$(basename $0) "
    echo "    [-h,--help] [--tool=auto|cmake|configure] [--build=<builddir>]"
    echo "    [-T,--type=<build-type] [--jobs=<make-jobs>]"
    echo "    [--cc=<c-compiler>] [--target=<src-root-dir>] [--cxx-flags=<flags>]"
    echo "    [-c,--clean] [-i,--interactive] [--dist]"
    echo "    [-s,--setup] [-r,--run] [-d,--debug] [-D,--data=<datadir>]"
    echo "    [-- [<Configure/CMake args]]"
    echo ""
    echo "   compiler: ${compiler} (cxxflags: ${cxx_flags})"
    echo "  make_jobs: ${make_jobs}"
    echo "  build_dir: ${builddir}"
    echo "     target: ${target}"
    echo "       tool: ${build_tool}"
    echo " build_type: ${build_type}  (Debug|Release|NativeRelease|Maintainer|RelWithDebInfo|MinSizeRel|Profiler|...)"
    echo "  priv_data ${priv_data}"
    echo ""
    exit ${_ret}
}
parse_opts() {
    local _arg _isconfigarg=
    for _arg in "$@"; do
        if test -n "${_is_configarg}"; then
            spec_config_args[${#spec_config_args[@]}]="${_arg}"
            continue
        fi
        case "${_arg}" in
            --) _is_configarg=yes;;
            -h|--help) show_help 0;;
            --build=*) builddir=${_arg#--build=};;
            --cc=*) compiler=${_arg#--cc=};;
            --cxx-flags=*) cxx_flags=${_arg#--cxx-flags=};;
            --jobs=*) make_jobs=${_arg#--jobs=};;
            --target=*) target=${_arg#--target=};;
            -T*|--type=*) build_type=${_arg#-T};build_type=${build_type#--type=};;
            --tool=*) build_tool=${_arg#--tool=};;
            --data=*|-D*) priv_data=${_arg#-D};priv_data=${priv_data#--data=};;
            --setup|-s) do_setup=yes;;
            --run|-r) do_run=yes;;
            --debug|-d) do_debug=yes;;
            --clean|-c) force_clean=yes;;
            --interactive|-i) interactive=yes;;
            --dist) do_dist=yes;;
            -*) echo "error: unknown option '${_arg}'."; show_help 1;;
            *) echo "error: unknown argument '${_arg}'."; show_help 1;;
        esac
    done
}
yesno() {
    local _c _ret=1
    while true; do
        test -n "$@" && printf -- "$@"
        read -s -n 1 _c
        case "${_c}" in
            y|Y) _ret=0; break ;;
            n|N) _ret=1; break;;
        esac
        printf -- "${_c}" | grep -q -E '^[-0-9a-zA-Z_]$' \
        && printf -- "${_c}"
        printf -- '\n'
    done
    return ${_ret}
}

unset config_args; declare -a config_args
add_config_args() { local _arg; for _arg in "$@"; do config_args[${#config_args[@]}]="${_arg}"; done; }

unset spec_config_args; declare -a spec_config_args
parse_opts "$@"

! tmp_compiler=$(which "${compiler}") \
|| ! test -x "${tmp_compiler}" \
&& { echo "error: compiler '${compiler}' not found."; exit 2; }

compiler=${tmp_compiler}

case "$(basename "${compiler}")" in
    clang*) spec_cc_flags="${clang_cc_flags}"; spec_cxx_flags="${clang_cxx_flags}"
            ;;
    *) spec_cc_flags="${gcc_cc_flags}"; spec_cxx_flags="${gcc_cxx_flags}"
            ;;
esac
cc_dir=$(dirname "${compiler}")
compiler=$(basename "${compiler}")
cc_pref=${compiler%%-*}
cc_suff=${compiler#${cc_pref}}

case "${cc_pref}" in
    [cg]"++"|cc|gcc) cc_pref=gcc; cxx_pref=g++;;
    *) cc_pref=${cc_pref%++}; cxx_pref=${cc_pref}++;;
esac

export CC="${cc_dir}/${cc_pref}${cc_suff}"
export CFLAGS="${spec_cc_flags} ${cc_flags}"
export CXX="${cc_dir}/${cxx_pref}${cc_suff}"
export CXXFLAGS="${spec_cxx_flags} ${cxx_flags}"

#### FIND SDK
_test_sdk=$(${CC} -v --version 2>&1 | sed -n -e 's%^[[:space:]]*\(/[^[:space:]]*SDKs/MacOSX[0-9].*\.sdk\)/[^[:space:]]*[[:space:]]*$%\1%p' | uniq)
sysver=$(uname -r)
if test ${sysver%%.*} -gt 15; then # 15 is capitan
    # sierra or later

    test -n "${_test_sdk}" && macos_sdk_fwk=${_test_sdk} \
        || macos_sdk_fwk="/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk"
    macos_sdk=${macos_sdk_fwk}
else
    # capitan or earlier
    #macos_sdk_fwk="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk"
    macos_sdk_fwk="/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk"
    macos_sdk=""
fi


#### START CONFIG & BUILD
echo "+ CC          ${CC}  (${CFLAGS})"
echo "+ CXX         ${CXX}  (${CXXFLAGS})"
echo "+ SDK         ${macos_sdk}"
echo "+ SDK_FWK     ${macos_sdk_fwk}"
echo "+ SRC         ${target}  (BUILD_DIR: ${builddir})"
echo "+ BUILDER     ${build_tool}  (TYPE: ${build_type})"
echo

### DIST / make tarball
if test -n "${do_dist}"; then
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

    tar cJ \
        --exclude "${mydirname}/vegastrike*/build" \
        --exclude "${mydirname}/vegastrike*/build-*" \
        --exclude "${mydirname}/engine*/build" \
        --exclude "${mydirname}/engine*/build-*" \
        --exclude "${mydirname}/tmp" \
        --exclude "**/.svn" --exclude "**/.git" --exclude "**/CVSROOT" \
        --exclude "**/*.sw?" --exclude "**/*~" --exclude "**/.\#*" \
        -f "${archive}" "${mydirname}" \
    && echo "+ archive created: ${archive} (`format_size "${archive}"`)"

    ret=$?
    popd .. >/dev/null 2>&1

    exit $ret
fi

### DELETE/CREATE BUILD IF NECESSARY AND GO INSIDE
target=$(cd "${target}"; pwd)
pushd "${target}" >/dev/null 2>&1 || exit 1

if test -d "${target}/${builddir}"; then
    # AUTODETECT BUILT_TOOL according to current build directory
    if test "${build_tool}" = "auto"; then
        { test -f "${target}/${builddir}/config.log" && build_tool=configure; } \
        || { test -d "${target}/${builddir}/CMakeFiles" && build_tool=cmake; }
    fi
    test -n "${interactive}" -o -n "${force_clean}" && yesno "*** ${builddir} exists. Remove it ? (y: remove, n: keep) " \
    && rm -Rf "${target}/${builddir}" && echo "+ ${builddir} deleted."
elif test "${build_tool}" = "auto"; then
    # AUTODETECT BUILT_TOOL according to CMakeLists.txt/configure.ac presence
    build_tools=
    test -f "${target}/CMakeLists.txt" && build_tools="${build_tools}${buid_tools:+ }cmake"
    test -f "${target}/configure.ac" && build_tools="${build_tools}${buid_tools:+ }configure"
    for b in ${build_preferred}; do
        if echo " ${build_tools} " | grep -E " ${b} "; then
            build_tool=${b};
            break ;
        fi
    done
    test "${build_tool}" = "auto" && build_tool=${build_preferred%% *}
fi

mkdir -p "${builddir}"

pushd "${builddir}" || exit 2

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

### CONFIG: CMAKE
echo "+ Build (${build_tool})"

case "${build_tool}" in
    cmake)
        #-DCMAKE_CXX_STANDARD=gnu++14
        export SDLDIR=/usr/local/specific/libsdl1
        add_config_args \
        -DCMAKE_VERBOSE_MAKEFILE=ON \
        -DCMAKE_FIND_FRAMEWORK=LAST \
        -DZLIB_ROOT=/usr \
        -DZLIB_INCLUDE_DIR=/usr/include \
        -DPYTHON_LIBRARY="${PYTHON_LIBRARY}" -DPYTHON_INCLUDE_DIR="${PYTHON_INCLUDE_DIR}" \
        -DBOOST_ROOT=/usr/local/specific/boost150 \
        -DBOOST_INTERNAL=1_50 \
        -DPNG_INCLUDE_DIRS=/usr/local/specific/libpng12/include -DPNG_LIBRARIES=/usr/local/specific/libpng12/lib/libpng12.dylib \
        -DVS_FIND_PREFIX_MORE_PATHS="/usr/local/specific/ffmpeg1 /usr/local/specific/libsdl1 /opt/local" \
        -DGLUT_INCLUDE_DIR="${macos_sdk_fwk}/System/Library/Frameworks/GLUT.framework/Headers/" \
        -DGLUT_LIBRARIES="${macos_sdk_fwk}/System/Library/Frameworks/GLUT.framework/GLUT.tbd" \
        -DOPENAL_LIBRARY="${macos_sdk_fwk}/System/Library/Frameworks/OpenAL.framework/Versions/A/OpenAL.tbd" \
        -DOPENAL_INCLUDE_DIR="${macos_sdk_fwk}/System/Library/Frameworks/OpenAL.framework/Versions/A/Headers" \
        -D_GNU_SOURCE=1 \
        -DHAVE_UNORDERED_MAP=0 -DHAVE_TR1_UNORDERED_MAP=0 \
        -DCMAKE_BUILD_TYPE="${build_type}" \
        -DCMAKE_OSX_SYSROOT="${macos_sdk}" \
        -DSDL_DISABLE=0 -DSDL_WINDOWING_DISABLE=1 \
        -DCMAKE_OSX_DEPLOYMENT_TARGET=10.11 \
        -DVS_STATIC_LIBS=0 \
        "${spec_config_args[@]}" \
        "${target}"

        printf -- "%s\n" "${config_args[@]}" | diff -q build-sh_config-cache - 2> /dev/null \
        || { echo "+ build config changed"; cmake "${config_args[@]}" \
             && printf -- "%s\n" "${config_args[@]}" > build-sh_config-cache; }

        ;;
    configure)
        case "$(echo "${build_type}" | tr "[:upper:]" "[:lower:]")" in
            release) debug_config_args="--disable-debug --enable-release=2";;
            nativerelease) debug_config_args="--disable-debug --enable-release=3";;
            debug) debug_config_args="--enable-debug";;
            maintainer) debug_config_args="--enable-debug";;
            relwithdebinfo) debug_config_args="--enable-debug --enable-release=2";;
            profiler) debug_config_args="--enable-debug --enable-profile";;
            *) debug_config_args="--enable-debug --enable-release=2";;
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
        #--with-python-inc="${PYTHON_INCLUDE_DIR}" --with-python-libs="$(dirname "${PYTHON_LIBRARY}")" \

        add_config_args \
            --with-macos-sdk="${macos_sdk_fwk}" \
            --with-png-inc=/usr/local/specific/libpng12/include --with-png-libs=/usr/local/specific/libpng12/lib \
            --with-ffmpeg-inc=/usr/local/specific/ffmpeg1/include --with-ffmpeg-libs=/usr/local/specific/ffmpeg1/lib \
            --with-jpeg-inc=/opt/local/include --with-jpeg-libs=/opt/local/lib \
             --with-sdl-prefix=/usr/local/specific/libsdl1 --with-sdl-exec-prefix=/usr/local/specific/libsdl1 \
            --enable-sdl --disable-sdl-windowing \
            --with-vorbis-inc=/opt/local/include --with-vorbis-libs=/opt/local/lib \
            --with-boost=1.50 \
            --disable-unordered-map \
            ${debug_config_args} \
            --enable-macosx-bundle \
            "${spec_config_args[@]}"

        printf -- "%s\n" "${config_args[@]}" | diff -q build-sh_config-cache - 2> /dev/null \
        && test -f Makefile \
        || { echo "+ build config changed"; "${target}/configure" "${config_args[@]}" \
             && printf -- "%s\n" "${config_args[@]}" > build-sh_config-cache; }

        ;;
    *) echo "!! error: unknown build tool '${build_tool}'."; exit 1;;
esac
test $? -eq 0 || exit 3

# necessary for vssetup to run
echo '.privgold100' >> setup/Version.txt

### BUILD : MAKE
test -n "${interactive}" && { yesno "*** Build? " || exit 4; }

${MAKE} VERBOSE=1 -j${make_jobs}
ret=$?

if test $ret -ne 0 -a -f CMakeFiles/vegastrike.dir/link.txt; then
    #linker="\\1"                       #use same linker
    linker="clang++ -stdlib=libstdc++"  #use clang++
    eval `cat CMakeFiles//vegastrike.dir/link.txt \
        | sed -e 's%^[[:space:]]*\([^[:space:]]*\)\(.*\)%'"${linker}"' -v \2 \1%' \
              -e 's%\([^[:space:]]\)/[^/]*$%\1/../lib/libstdc++.a%'`
    ret=$?
fi

if test $ret -eq 0; then
    if test -n "${do_run}" -o -n "${do_debug}" -o -n "${do_setup}"; then
        build=$(cd "${target}/${builddir}";pwd)
        data=$(cd "${priv_data}";pwd)
        delay=20
        log="${mydir}/logs/log"
        mkdir -p $(dirname "${log}")

        echo "BUILD $build DATA $data"

        # important the chdir to have music and avoid crash when loading universe
        pushd "${data}" || exit 1

        if test -n "${do_setup}"; then
            "${build}/setup/vssetup" || "${build}/setup/vssetup_dlg" || "${build}/vssetup" || "${build}/vssetup_dlg"; ret=$?
        elif test -n "${do_debug}"; then # -D${data}
            ( unset PYTHON PYTHONHOME PYTHONPATH; lldb "${build}/vegastrike" \
                --batch -o "run" \
                --one-line-on-crash 'thread backtrace all' --one-line-on-crash 'quit' 2>&1;) | tee "${log}"
            #{ printf -- "run -D${data}\nbt\n"; sleep ${delay}; printf -- 'bt\n'; sleep 2; exit; } \
            #  |  { unset PYTHON PYTHONHOME PYTHONPATH; lldb "${build}/vegastrike" 2>&1; } | tee "${mydir}/${log}" & privpid=$!; ret=$?
        else
            "${build}/vegastrike" 2>&1 | tee "${mydir}/log"; ret=$?
        fi

        popd #data
    fi
fi

popd # build
popd # target

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

