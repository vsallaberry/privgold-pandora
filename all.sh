#!/bin/bash
do_clean=
buildpool=./build
unset buildargs; declare -a buildargs
for arg in "$@"; do
    case "$arg" in
        -h|--help) echo "$(basename "$0") [-h,--help] [-C,--clean] [build1 [build2 [...]]]"; exit 0;;
        -C|--clean) do_clean=yes;;
        -*) buildargs[${#buildargs[@]}]="$arg";;
        *) build_list="${build_list}${build_list:+ }${arg}";;
    esac
done
test -z "${build_list}" && build_list="sdl2 sdl2dbg sdl2fulldbg sdl1 glut auto univ gcc11 gcc8 gcc6 gcc5 native clang12 gcc6dbg glut1"
for f in $build_list; do
    tool=auto;cxxf=;args=;mcc=;gfx=sdl2;type=release
    unset newbuildargs; declare -a newbuildargs
    case $f in
        glut1) args="--optimize=-Os";gfx=glut1;;
        sdl2) args="--optimize=-Os";gfx=sdl2;;
        sdl2dbg) type=RelWithDebInfo;args="--optimize=-O1";gfx=sdl2;;
        sdl2fulldbg) type=Debug;gfx=sdl2;;
        native) type=NativeRelease;args="--optimize=-O3";
                newbuildargs[${#newbuildargs[@]}]="--cxx-flags=-ffast-math";gfx=sdl2;;
        auto) tool=configure;;
        univ) newbuildargs[${#newbuildargs[@]}]="--cxx-flags=-arch i386 -arch x86_64"; args="--optimize=-Os";;
        gcc6) mcc=gcc6; args=--cxx-std=98;;
        gcc6dbg) mcc=gcc6; args="--cxx-std=98 --optimize=-O1"; type=RelWithDebInfo;;
        gcc5) mcc=gcc-mp-5; args=--cxx-std=98; tool=configure;;
        gcc11) mcc=gcc-mp-11;;
        clang12) mcc=clang-mp-12;;
        gcc8) mcc=gcc-mp-8; tool=configure;;
        *) gfx=$f;;
    esac
    test -z "${do_clean}" -a -d "${buildpool}/$f" -a -f "${buildpool}/$f/.vs_build-sh_config-cache" && args= \
    || args="--clean --tool=$tool --gfx=$gfx --type=${type} ${mcc:+"--cc=${mcc}"}${args:+ }${args}"
    ./build.sh -b${buildpool}/$f $args "${newbuildargs[@]}" "${buildargs[@]}" || break
done
