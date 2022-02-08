#!/bin/bash
##############################################################################
# Copyright (C) 2021 Vincent Sallaberry
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

usage() {
    echo "$0 [-NYUSV] <arch1_dev_dir> ... <archN_dev_dir> <dst_root_dir>"
    echo "   use make install DESTDIR=<tmpdesroot> for each target then merge archs into dst_root_dir."
    echo "   -N: skip archs_dev_die installs"
    echo "   -Y: skip archs_dev_dirs copy in destroot, just merging libs in destroot"
    echo "   -U: pick up all architectures in each file"
    echo "   -S: don't add the destroot suffix"
    echo "   -V: verbose, can be used multiple times"
    echo "   -X<path>: exclude path when copying to destroot"
    echo "   the first <arch_dev_dir> has allways priority."
    exit $1
}

test -z "$1" && usage 1

do_destroot=yes
do_install=yes
destroot_suffix=/destroot
take_all=
verbose=0

declare -a builddirs
declare -a excludes

while test -n "$1"; do
    case "$1" in
        -N) do_install=;;
        -Y) do_destroot=;;
        -U) take_all=yes;;
        -S) destroot_suffix=;;
        -V) verbose=$((verbose+1));;
        -X*)excludes[${#excludes[@]}]="--exclude"; excludes[${#excludes[@]}]=${1#-X};;
        -*) ;;
        *) test -z "$2" && destroot="$1" || builddirs[${#builddirs[@]}]="$1";;
    esac
    shift
done

test -n "${destroot_suffix}" -o -z "${do_install}" \
    || { echo "!! options -S (nosuffix) must be used with option -N (no install)"; exit 1; }

test -n "$destroot" || { echo "!! no destroot specified"; usage 1; }

for d in "${builddirs[@]}"; do
    test -d "$d" || { echo "!! wrong builddir '$d'"; usage 1; }
done

umask 022

for d in "${builddirs[@]}"; do
    mkdir -p "${d}${destroot_suffix}" || { echo "!! cannot create '${d}${destroot_suffix}"; usage 2; }
done

mkdir -p "$destroot" || { echo "!! cannot create destroot '${destroot}'"; usage 2; }

pushd >/dev/null 2>&1 "$destroot" && destroot=`pwd` && popd >/dev/null 2>&1 \
|| { echo "!! bad destroot directory '$destroot'"; usage 3; }

lipo=lipo
for f in $(which -a lipo /opt/local/bin/lipo 2> /dev/null); do
    "${f}" -archs "${f}" >/dev/null 2>&1 && { lipo=$f; break ; }
done

xcopy() {
    rsync -ah -t "${excludes[@]}" "$@"
}
declare -a build_destroot
for ((i=0; i < ${#builddirs[@]}; i=i+1)); do
    pushd >/dev/null 2>&1 "${builddirs[$i]}" && builddirs[$i]=`pwd` && popd >/dev/null 2>&1 \
    || { echo "!! bad directories"; usage 3; }
    test "${builddirs[$i]}" = "${destroot}" && { echo "!! folders must be distinct"; usage 4; }
    build_destroot[${#build_destroot[@]}]="${builddirs[$i]}${destroot_suffix}"
done

if test -n "$do_install"; then
    for d in "${builddirs[@]}"; do
        echo "+ installing '$d'"
        make -C "$d" install DESTDIR="${d}${destroot_suffix}" || exit 5
    done
fi

test "${verbose}" -gt 1 && { echo "** DIFF"; diff -qru "${build_destroot[@]}"; echo; }

if test -n "${do_destroot}"; then
    echo "+ ${builddirs[@]} installed. copying into $destroot..."
    echo

    first=
    for d in "${builddirs[@]}"; do
        test -z "${first}" && { first="${d}${destroot_suffix}/"; continue; }
        xcopy "${d}${destroot_suffix}/" "$destroot"
    done
    test -n "${first}" && { echo "+ Finally copying main_arch: '${first}'"; xcopy "${first}" "$destroot"; }
fi

echo "+ $destroot created, Merging libs..."

declare -a patterns
for b in "${builddirs[@]}"; do
    patterns[${#patterns[@]}]="-e"; patterns[${#patterns[@]}]="s|^${b}${destroot_suffix}/||"
done
files=`find "${build_destroot[@]}" \
            -not -type l \
            -and \( -iname '*.dylib' -o -iname '*.a' -o -iname '*.so' -o -iname '*.dll' \
                    -o -iname '*.dylib.[0-9]*' -o -iname '*.so.[0-9]*' \
                    -o \( -not -type d -and -perm '+u=x' \) \
                 \) \
                 | sed "${patterns[@]}" | sort | uniq`

test "${verbose}" -gt 1 && printf "FILES:\n$files\n\n"

for f in $files; do
    filedst="${destroot}/$f"
    ref_archs=

    unset lipo_args; declare -a lipo_args
    add_lipo() { for a in "$@"; do lipo_args[${#lipo_args[@]}]="$a"; done }

    for b in "${builddirs[@]}"; do
        filesrc="${b}${destroot_suffix}/${f}"
        test -e "${filesrc}" || { echo "!! ${f} not found in build $(basename "`dirname "${b}"`")/$(basename "${b}")"; continue; }
        arch=$("${lipo}" -archs "${filesrc}" 2> /dev/null)
        test -n "${arch}" || { echo "!! $f: not a valid binary"; continue; }
        #lipo "$file32" -verify_arch i386 >/dev/null 2>&1 && lipo "$file64" -verify_arch x86_64 >/dev/null 2>&1 || { echo "!! one of 32/64 file has not requested arch"; exit 6; }

        mkdir -p "`dirname "${filedst}"`" || { echo "!! cannot create dir for $filedst"; exit 7; }

        if test -n "${take_all}" -a -z "${ref_archs}"; then
            ref_archs=${arch}; arch=all
            cp -a "${filesrc}" "${filedst}.tmp.${arch}"
        else
            if test -n "${ref_archs}"; then
                curarch=${arch}; arch=
                for a in ${curarch}; do
                    case " ${ref_archs} " in *" ${a} "*) ;; *) arch="${arch}${arch:+ }${a}";; esac
                done
                test -z "${arch}" && continue
            fi
            for a in ${arch}; do
                test "${verbose}" -gt 0 && echo "extract '$arch' for '${filesrc}'"
                "${lipo}" -extract "${a}" "${filesrc}" -output "${filedst}.tmp.${a}" >/dev/null 2>&1 || cp -a "${filesrc}" "${filedst}.tmp.${a}"
            done
        fi
        test "${arch}" = "all" && add_lipo "${filedst}.tmp.${arch}" \
        || for a in ${arch}; do add_lipo "-arch" "${a}" "${filedst}.tmp.${a}"; done
    done
    test "${#lipo_args[@]}" -eq 0 && continue

    #if test -e "${filedst}"; then
    #    i=0; suf=; while test -e "${filedst}$suf"; do suf=".$i"; i=$((i+1)); done
    #    cp -a "$filedst" "${filedst}$suf"
    #fi

    test "${verbose}" -gt 1 && echo ">> ${lipo}" "${lipo_args[@]}" -create -output "${filedst}"
    "${lipo}" "${lipo_args[@]}" -create -output "${filedst}" \
        && { printf -- "+ ${filedst#${destroot}}: "; "${lipo}" -archs "${filedst}"; } && rm -f "${filedst}.tmp."*  \
        || { echo "!! lipo could not create '${filedst}'"; exit 8; }

done


