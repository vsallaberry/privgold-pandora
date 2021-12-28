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
    echo "$0 [-NYU] <arch1_dev_dir> ... <archN_dev_dir> <dst_root_dir>"
    echo "   use make install DESTDIR=<tmpdesroot> for each target then merge archs into dst_root_dir."
    echo "   -N: skip archs_dev_die installs"
    echo "   -Y: skip archs_dev_dirs copy in destroot, just merging libs in destroot"
    echo "   -U: pick up all architectures in each file"
    echo "   the first <arch_dev_dir> has allways priority."
    exit $1
}

test -z "$1" && usage 1

do_install=yes
do_destroot=yes
take_all=

declare -a builddirs

while test -n "$1"; do
    case "$1" in
        -N) do_install=;;
        -Y) do_destroot=;;
        -U) take_all=yes;;
        -*) ;;
        *) test -z "$2" && destroot="$1" || builddirs[${#builddirs[@]}]="$1";;
    esac
    shift
done

test -n "$destroot" || { echo "!! no destroot specified"; usage 1; }

for d in "${builddirs[@]}"; do
    test -d "$d" || { echo "!! wrong builddir '$d'"; usage 1; }
done

umask 022

for d in "${builddirs[@]}"; do
    mkdir -p "${d}/destroot" || { echo "!! cannot create '${d}/destroot"; usage 2; }
done

mkdir -p "$destroot" || { echo "!! cannot create destroot '${destroot}'"; usage 2; }

pushd >/dev/null 2>&1 "$destroot" && destroot=`pwd` && popd >/dev/null 2>&1 \
|| { echo "!! bad destroot directory '$destroot'"; usage 3; }

declare -a build_destroot
for ((i=0; i < ${#builddirs[@]}; i=i+1)); do
    pushd >/dev/null 2>&1 "${builddirs[$i]}" && builddirs[$i]=`pwd` && popd >/dev/null 2>&1 \
    || { echo "!! bad directories"; usage 3; }
    test "${builddirs[$i]}" = "${destroot}" && { echo "!! folders must be distinct"; usage 4; }
    build_destroot[${#build_destroot[@]}]="${builddirs[$i]}/destroot"
done

if test -n "$do_install"; then
    for d in "${builddirs[@]}"; do
        echo "+ installing '$d'"
        make -C "$d" install DESTDIR="${d}/destroot" || exit 5
    done
fi

echo "+ ${builddirs[@]} installed. copying into $destroot..."
echo

diff -qru "${build_destroot[@]}"
echo

if test -n "${do_destroot}"; then
    first=
    for d in "${builddirs[@]}"; do
        test -z "${first}" && { first="${d}/destroot/"; continue; }
        cp -a "${d}/destroot/" "$destroot"
    done
    test -n "${first}" && { echo "+ Finally copying main_arch: '${first}'"; cp -a "${first}" "$destroot"; }
fi

echo "+ $destroot created, Merging libs..."

declare -a patterns
for b in "${builddirs[@]}"; do
    patterns[${#patterns[@]}]="-e"; patterns[${#patterns[@]}]="s|^${b}/destroot/||"
done
files=`find "${build_destroot[@]}" \
            -not -type l \
            -and \( -iname '*.dylib' -o -iname '*.a' -o -iname '*.so' -o -iname '*.dll' \
                    -o -iname '*.dylib.[0-9]*' -o -iname '*.so.[0-9]*' \
                    -o \( -not -type d -and -perm '+u=x' \) \
                 \) \
                 | sed "${patterns[@]}" | sort | uniq`

printf "FILES:\n$files\n\n"

for f in $files; do
    filedst="${destroot}/$f"
    ref_archs=

    unset lipo_args; declare -a lipo_args
    add_lipo() { for a in "$@"; do lipo_args[${#lipo_args[@]}]="$a"; done }

    for b in "${builddirs[@]}"; do
        filesrc="${b}/destroot/${f}"
        test -e "${filesrc}" || { echo "!! ${f} not found in build $(basename "`dirname "${b}"`")/$(basename "${b}")"; continue; }
        arch=$(lipo -archs "${filesrc}")
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
                    case " ${ref_archs} " in *" ${a} "*) ;; *) arch="${arch:+ }${a}";; esac
                done
                test -z "${arch}" && continue
            fi
            lipo -extract "${arch}" "${filesrc}" -output "${filedst}.tmp.${arch}" >/dev/null 2>&1 || cp -a "${filesrc}" "${filedst}.tmp.${arch}"
        fi
        test -n "${take_all}" && add_lipo "${filedst}.tmp.${arch}" \
        || add_lipo "-arch" "${arch}" "${filedst}.tmp.${arch}"
    done

    test "${#lipo_args[@]}" -eq 0 && continue

    #if test -e "${filedst}"; then
    #    i=0; suf=; while test -e "${filedst}$suf"; do suf=".$i"; i=$((i+1)); done
    #    cp -a "$filedst" "${filedst}$suf"
    #fi

    lipo "${lipo_args[@]}" -create -output "${filedst}" \
        && { printf -- "+ ${filedst#${destroot}}: "; lipo -archs "${filedst}"; } && rm -f "${filedst}.tmp."*  \
        || { echo "!! lipo could not create '${filedst}'"; exit 8; }

done


