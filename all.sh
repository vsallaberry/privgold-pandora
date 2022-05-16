#!/bin/bash
do_clean=
buildpool=./build
build_sysname=$(uname -s | tr "[:upper:]" "[:lower:]")
build_sysmajor=$(uname -r | awk -F '.' '{ print $1 }')
case "${build_sysname}" in
    darwin*) build_dllext=.dylib;;
    mingw*|cywin*|msys*|windows*) build_dllext=.dll.a;;
    *) build_dllext=.so;;
esac
unset buildargs; declare -a buildargs
for arg in "$@"; do
    case "$arg" in
        -h|--help) echo "$(basename "$0") [-h,--help] [-C,--clean] [build1 [build2 [...]]]"; exit 0;;
        -C|--clean) do_clean=yes;;
        -*) buildargs[${#buildargs[@]}]="$arg";;
        *) build_list="${build_list}${build_list:+ }${arg}";;
    esac
done
test -z "${build_list}" && build_list="sdl2 sdl2dbg sdl2fulldbg sdl1 glut auto univ gcc11 gcc8 gcc6 gcc5 native clang12 gcc6dbg glut1 png12 sdl2-32 sdl2dbg-32"
for f in $build_list; do
    tool=auto; cxxf=; args=; mcc=; gfx=sdl2; type=release
    unset newbuildargs; declare -a newbuildargs
    unset configargs; declare -a configargs;configargs[${#configargs[@]}]="--"
    case "$f" in
        glut1) args="--optimize=-O3";gfx=glut1;;
        sdl2) args="--optimize=-O3";gfx=sdl2;;
        sdl2dbg) type=RelWithDebInfo;args="--optimize=-O1";gfx=sdl2;;
        sdl2-32) args="--optimize=-O3";gfx=sdl2
                 case "${build_sysname}" in 
                     bsd*|linux*) newbuildargs[${#newbuildargs[@]}]="--cxx-flags=-m32 -D_GLIBCXX_USE_CXX11_ABI=0"; args="${args} --cxx-std=98";;
                     mingw*|cywin*|msys*|windows*) 
                        newbuildargs[${#newbuildargs[@]}]="--cc=/mingw32/bin/clang"
                        newbuildargs[${#newbuildargs[@]}]="--cxx-flags=-D_WIN32_WINNT=0x0501";;
                     *) continue ;;
                 esac;;
        sdl2dbg-32) type=RelWithDebInfo;args="--optimize=-O1";gfx=sdl2
                    case "${build_sysname}" in 
                        bsd*|linux*) newbuildargs[${#newbuildargs[@]}]="--cxx-flags=-m32 -D_GLIBCXX_USE_CXX11_ABI=0"; args="${args} --cxx-std=98";;
                        mingw*|cywin*|msys*|windows*) 
                            newbuildargs[${#newbuildargs[@]}]="--cc=/mingw32/bin/clang"
                            newbuildargs[${#newbuildargs[@]}]="--cxx-flags=-D_WIN32_WINNT=0x0501";;
                        *) continue ;;
                    esac;;
        sdl2fulldbg) type=Debug;gfx=sdl2;;
        native) type=NativeRelease;args="--optimize=-O3";
                newbuildargs[${#newbuildargs[@]}]="--cxx-flags=-ffast-math";gfx=sdl2;;
        auto) tool=configure;;
        univ) case "${build_sysname}" in 
                darwin*) test ${build_sysmajor} -lt 19 && archs="-arch i386 -arch x86_64" \
                     || archs="-arch arm64 -arch x86_64";; 
                *) continue;; 
              esac
              newbuildargs[${#newbuildargs[@]}]="--cxx-flags=${archs}"; args="--optimize=-O3";;
        gcc6) mcc=gcc6; args="--cxx-std=98 --cxx-flags=-Wno-strict-aliasing";;
        gcc6dbg) mcc=gcc6; args="--cxx-std=98 --optimize=-O1 --cxx-flags=-Wno-strict-aliasing"; type=RelWithDebInfo;;
        gcc5) mcc=gcc-mp-5; args="--cxx-std=98 --cxx-flags=-Wno-strict-aliasing"; tool=configure;;
        gcc11) mcc=gcc-mp-11; args="--cxx-flags=-Wno-strict-aliasing";;
        clang12) mcc=clang-mp-12;;
        gcc8) mcc=gcc-mp-8; tool=configure;;
        png12) pngpref="/usr/local/specific/libpng12"; test -d "${pngpref}" || continue
               args="--optimize=-O3";gfx=sdl2; 
               configargs[${#configargs[@]}]="-DFFMPEG_INCLUDE_DIRS=${pngpref}/include/libpng12;/usr/local/vega05/include"
               configargs[${#configargs[@]}]="-DFFMPEG_LIBRARIES=/usr/local/vega05/lib/libavutil${build_dllext};/usr/local/vega05/lib/libavcodec${build_dllext};/usr/local/vega05/lib/libavformat${build_dllext};/usr/local/vega05/lib/libswscale${build_dllext};/usr/local/vega05/lib/libbz2${build_dllext}"
               for define in HAVE_LIBAVCODEC_AVCODEC_H HAVE_LIBAVCODEC_AVUTIL_H HAVE_LIBAVFORMAT_AVFORMAT_H HAVE_LIBSWSCALE_SWSCALE_H HAVE_LIBAVFILTER_AVFILTER_H HAVE_LIBAVFORMAT_AVIO_H HAVE_LIBAVDEVICE_AVDEVICE_H HAVE_LIBSWRESAMPLE_SWRESAMPLE_H; do
                   configargs[${#configargs[@]}]="-D${define}=1"
               done
               configargs[${#configargs[@]}]="-DPNG_INCLUDE_DIRS=${pngpref}/include/libpng12"
               configargs[${#configargs[@]}]="-DPNG_LIBRARIES=${pngpref}/lib/libpng12${build_dllext}";;
        *) gfx=$f;;
    esac
    test -z "${mcc}" -o -x "$(which "${mcc}")" || { echo "!! compiler '${mcc}' not available on this system, skipping..."; continue; }
    test -z "${do_clean}" -a -d "${buildpool}/$f" -a -f "${buildpool}/$f/.vs_build-sh_config-cache" && { args=; unset configargs newbuildargs; } \
    || args="--clean --tool=$tool --gfx=$gfx --type=${type} ${mcc:+"--cc=${mcc}"}${args:+ }${args}"
    ./build.sh -b${buildpool}/$f $args "${newbuildargs[@]}" "${buildargs[@]}" "${configargs[@]}" || break
done
