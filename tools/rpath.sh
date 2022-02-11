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
unset targets; declare -a targets
unset excludes; declare -a excludes
unset rpaths; declare -a rpaths
libs_path="../Resources/lib"
exe_rpath="@executable_path/"
rpaths[0]=${libs_path}
dostrip=

build_sysname=$(uname -s | tr "[:upper:]" "[:lower:]")
build_sysmajor=$(uname -r | awk -F '.' '{ print $1 }')

##################################################################################
show_help() {
    echo "`basename $0` file [-L<libs-rel-path>] [-X<exclude-lib-path>] [-R<more-rpath] [-S] [bin [bin [...]]]"
    echo "  change libpaths to <libs-rel-path>=${libs_path}"
    echo "  -X<exclude-lib-path>    : dont check or copy libs from <exclude-lib-path>"
    echo "  -R<more-rpaths>         : add more @rpath"
    echo "  -S                      : strip binaries and libraries"
    exit $1
}
rstrip_slash() {
    local _arg=$@
    while case "$_arg" in */) true;; *) false;; esac; do _arg=${_arg%/}; done
    printf -- "${_arg}"
}
for arg in "$@"; do
    case "${arg}" in
        -h) show_help 0;;
        -L*) libs_path=${arg#-L}; libs_path=$(rstrip_slash "${libs_path}");;
        -X*) arg=${arg#-X}; excludes[${#excludes[@]}]=$(rstrip_slash "${arg}");;
        -R*) arg=${arg#-R};   rpaths[${#rpaths[@]}]=$(rstrip_slash "${arg}");;
        -S)  dostrip=yes;;
        -*) echo "!! unknown option '${arg}'"; exit 1;;
        *) targets[${#targets[@]}]="${arg}";;
    esac
done

case "${libs_path}" in
    /*) echo "!! error '${libs_path}': rpath cannot be absolute"; exit 1;;
esac

rpaths[0]=${libs_path}

case "${build_sysname}" in
    linux*|*bsd*|mingw*|cygwin*|msys*)
        exe_rpath=
        chrpath=$(which chrpath 2> /dev/null)
        install_name_tool() { 
            test -x ${chrpath} || return 0
            case "$1" in
                -add_rpath) local rpath=$2 target=$3; chrpath -r "${rpath}" "${target}";;
            esac
        }
        get_libs() { ldd "$@" | awk '/[^:]$/ { print $3 }'; }
        ;;
    darwin*)
        get_libs() { otool -L "$@" | awk '/[^:]$/ { print $1 }'; }
        ;;
esac

printf -- "\n**********************************************\n"
printf -- "* running $(basename "$0"), strip:${dostrip}\n"
printf -- "  + targets:\n"
printf -- "    %s\n" "${targets[@]}"
printf -- "  + main lib rpath:\n    ${libs_path}\n"
printf -- "  + other rpaths:\n"
printf -- "    %s\n" "${rpaths[@]}"
printf -- "  + excludes:\n"
printf -- "    %s\n" "${excludes[@]}"
printf -- "\n"

##################################################################################
unset errors; declare -a errors
unset warnings; declare -a warnings
i=0; while test $i -lt ${#targets[@]}; do target=${targets[$i]}; i=$((i+1))
    echo "+ target [${target}]"
    target_name=$(basename "${target}")
    lowtarget=$(echo "${target}" | tr "[:upper:]" "[:lower:]")

    test -f "${target}" || { echo "!! target '${target_name}' not found"; errors[${#errors[@]}]="[${target_name}] file not found!"; continue ; }

    if test -z "${target_libdir}"; then
        target_dir=$(dirname "${target}")
        target_libdir="${target_dir}/${libs_path}"
    fi

    case "${lowtarget}" in
        *.dylib|*.dylib.[0-9]*|*.so|*.so.[0-9]*|*.dll|/frameworks/*)
            ;;
        *)
            chmod 'u+w,u+r' "${target}"
            for rpath in "${rpaths[@]}"; do
                case "${rpath}" in /*) new_rpath=${rpath};; *) new_rpath="${exe_rpath}${rpath}";; esac
                install_name_tool -add_rpath "${new_rpath}" "${target}" 2> /dev/null \
                || install_name_tool -rpath "${new_rpath}" "${new_rpath}" "${target}" \
                || errors[${#errors[@]}]="[${target_name}] exe rpath add/update"
            done
            test -z "${dostrip}" || { strip -S "${target}" && echo "+ $target stripped." || errors[${#errors[@]}]="strip $target"; }
            ;;
    esac

    for lib in `get_libs "${target}"`; do
        #echo "** ${lib}"
        lib_name=$(basename "${lib}")
        lowlib=$(echo "${lib}" | tr "[:upper:]" "[:lower:]")

        case "${lowlib}" in
            /usr/lib/*) ;;
            /system/library/*) ;;
            /lib/*|/lib64/*|/lib32/*) ;;
            /c/windows/*) ;;
            @*) install_name_tool -change "${lib}" "@rpath/${lib_name}" "${target}" \
                    || errors[${#errors[@]}]="[${target_name}] nametool change @ (${lib_name})";;
            *)  copy=yes; for x in "${excludes[@]}"; do
                    case "${lib}" in "${x}"/*) copy=; warnings[${#warnings[@]}]="ignoring $lib (excludes)"; break;; esac
                done
                for rp in "${rpaths[@]}"; do
                    test -f "${target_dir}/${rp}/${lib_name}" && { copy=; break; }
                done
                install_name_tool -change "${lib}" "@rpath/${lib_name}" "${target}" \
                    || errors[${#errors[@]}]="[${target_name}] nametool change * (${lib_name})"
                test -z "${copy}" \
                || if mkdir -p "${target_libdir}" && cp -Lv "${lib}" "${target_libdir}"; then
                    chmod 'u+w,u+r' "${target_libdir}/${lib_name}"
                    install_name_tool -id "@rpath/${lib_name}" "${target_libdir}/${lib_name}" \
                        && { test -z "${dostrip}" || { strip -S "${target_libdir}/${lib_name}" && echo "+ ${lib_name} stripped"; }; } \
                        || errors[${#errors[@]}]="[${target_name}] nametool id (${lib_name})"
                    targets[${#targets[@]}]="${target_libdir}/${lib_name}"

                else
                    errors[${#errors[@]}]="[${target_name}] cp '$lib'"
                fi
                ;;
        esac
    done
done

echo
if test ${#warnings[@]} -gt 0; then
    printf -- '+ warning: %s\n' "${warnings[@]}"
    echo
fi
if test ${#errors[@]} -eq 0; then
    echo "+ done"
else
    echo "errors:"
    printf -- '!! %s\n' "${errors[@]}"
fi
echo

