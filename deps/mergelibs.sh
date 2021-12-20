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
    echo "$0 [-NY] <32b_dev_dir> <64b_dev_dir> <dst_root_dir>"
    echo "   use make install DESTDIR=<tmpdesroot> for each target then merge archs into dst_root_dir."
    echo "   -N: skip dev32 and dev64 install"
    echo "   -Y: skip dev32 and dev64 copy in destroot, just merging libs in destroot"
    exit $1
}

test -z "$1" && usage 1

do_install=yes
do_destroot=yes

while test -n "$1"; do
    case "$1" in
        -N) do_install=;;
        -Y) do_destroot=;;
        -*) ;;
        *) break ;;
    esac
    shift
done

dev32=$1
dev64=$2
destroot=$3

test -d "$dev32" -a -d "$dev64" -a -n "$destroot" || { echo "!! wrong input"; usage 1; }

umask 022

mkdir -p "$dev32/destroot" "$dev64/destroot" "$destroot" || { echo "!! cannot create destroots"; usage 2; }

pushd >/dev/null 2>&1 "$dev32" && dev32=`pwd` && popd >/dev/null 2>&1 \
&& pushd >/dev/null 2>&1 "$dev64" && dev64=`pwd` && popd >/dev/null 2>&1 \
&& pushd >/dev/null 2>&1 "$destroot" && destroot=`pwd` && popd >/dev/null 2>&1 \
|| { echo "!! bad directories"; usage 3; }

test "$dev32" = "$dev64" -o "$dev32/destroot" = "$destroot" && { echo "!! folders must be distinct"; usage 4; }

if test -n "$do_install"; then
    for d in "$dev32" $dev64; do
        echo "+ installing '$d'"
        make -C "$d" install DESTDIR="${d}/destroot" || exit 5
    done
fi

echo "+ $dev32 and $dev64 installed. copying into $destroot..."

diff -qru "${dev32}/destroot" "${dev64}/destroot"

if test -n "${do_destroot}"; then
    cp -a "$dev32/destroot/" "$destroot"
    cp -a "$dev64/destroot/" "$destroot"
fi

echo "+ $destroot created, Merging libs..."

files=`find "$dev64/destroot" "$dev32/destroot" \
            -not -type l \
            -and \( -iname '*.dylib' -o -iname '*.a' -o -iname '*.so' -o -iname '*.dll' \
                    -o -iname '*.dylib.[0-9]*' -o -iname '*.so.[0-9]*' \
                    -o \( -not -type d -and -perm '+u=x' \) \
                 \) \
        | sed -e "s|^${dev32}/destroot/||" -e "s|^${dev64}/destroot/||" | sort | uniq`

printf "FILES:\n$files\n"

for f in $files; do
    file32="${dev32}/destroot/$f"
    file64="${dev64}/destroot/$f"
    filedst="${destroot}/$f"

    test -e "$file32" -a -e "$file64" || { echo "!! 32 or 64 file not found for $f"; continue; }
    lipo -info "$file32" >/dev/null 2>&1 && lipo -info "$file64" >/dev/null 2>&1 || { echo "!! $f: not a valid binary"; continue; }
    lipo "$file32" -verify_arch i386 >/dev/null 2>&1 && lipo "$file64" -verify_arch x86_64 >/dev/null 2>&1 || { echo "!! one of 32/64 file has not requested arch"; exit 6; }

    mkdir -p "`dirname "${filedst}"`" || { echo "!! cannot create dir for $filedst"; exit 7; }

    lipo -extract i386   "$file32" -output "${filedst}.32" >/dev/null 2>&1 || cp -a "$file32" "${filedst}.32"
    lipo -extract x86_64 "$file64" -output "${filedst}.64" >/dev/null 2>&1 || cp -a "$file64" "${filedst}.64"

    #if test -e "${filedst}"; then
    #    i=0; suf=; while test -e "${filedst}$suf"; do suf=".$i"; i=$((i+1)); done
    #    cp -a "$filedst" "${filedst}$suf"
    #fi

    lipo -arch x86_64 "${filedst}.64" -arch i386 "${filedst}.32" -create -output "${filedst}" \
    && echo "+ ${filedst}" && rm -f "${filedst}.64" "${filedst}.32" \
    || { echo "!! lipo could not create '${filedst}'"; exit 8; }

done


