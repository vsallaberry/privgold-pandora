#!/bin/bash
##############################################################################
# Copyright (C) 2021-2022 Vincent Sallaberry
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
unset maintargets; declare -a maintargets
unset excludes; declare -a excludes
unset rpaths; declare -a rpaths
unset exec_paths; declare -a exec_paths
libs_path="../Resources/lib"
exe_rpath="@executable_path"
rpaths[0]=${libs_path}
dostrip=
loglevel=1
do_tests=
exec_path=

build_sysname=$(uname -s | tr "[:upper:]" "[:lower:]")
build_sysmajor=$(uname -r | awk -F '.' '{ print $1 }')

##################################################################################
show_help() {
    echo "`basename $0` file [-L<libs-path>] [-X<exclude-lib-path>] [-R<more-rpath]"
    echo "              [-E<default-executable>] [-V] [-S] [bin [bin [...]]]"
    echo "  change libpaths to <libs-path>=${libs_path}"
    echo "  <libs-path> and <more-rpath> can be absolute. Prefix with ':' to take is as given"
    echo "  -X<exclude-lib-pattern> : dont check or copy libs from <exclude-lib-pattern>"
    echo "  -R<more-rpaths>         : add more @rpath (can be repeated)"
    echo "  -E<executable-path>     : additional executable path (can be repeated)"
    echo "  -S                      : strip binaries and libraries"
    echo "  -V                      : verbose (can be repeated)"
    exit $1
}
log() {
    local level=$1; shift
    test ${loglevel} -ge ${level} && echo "$@" 1>&2
}
rstrip_slash() {
    local _arg=$@
    while case "$_arg" in */) true;; *) false;; esac; do _arg=${_arg%/}; done
    printf -- "${_arg}"
}
abspath() {
    local path=$1 newpath
    case "${path}" in /*) ;; *) path="$(pwd)/${path}";; esac
    while true; do
        while test "${path%%/*}" = "."; do path="${path#./}"; done
        while test "${path%%/*}" = ".."; do path="${path#../}"; newpath="${newpath%/*/}/"; done
        newpath="${newpath}${path%%/*}";
        case "$path" in */*) newpath="${newpath}/"; path="${path#*/}";; *) break;; esac
    done
    echo "${newpath}"
}
stripslashes() {
    local path=$1 newpath=
    while case "${path}" in */) true;; *) false;; esac; do path=${path%/}; done
    while true; do
        newpath="${newpath}${path%%/*}";
        case "$path" in */*) newpath="${newpath}/"; path="${path#*/}";; *) break;; esac
        while case "${path}" in /*) true;; *) false;; esac; do path="${path#/}"; done
    done
    echo "${newpath}"
}
relpath() {
    local path=$1 base=$2 file tmppath common bpref newpath newbase rpath
    test -f "${base}" && base=$(dirname "${base}")
    test -f "${path}" && { file=$(basename "${path}"); path=$(dirname "${path}"); }
    base=$(cd "${base}" 2>/dev/null && pwd || echo "$(abspath "$(stripslashes "${base}")")")
    path=$(cd "${path}" 2>/dev/null && pwd || echo "$(abspath "$(stripslashes "${path}")")")
    case "${path}" in
        "${base}") rpath=;;
        "${base}"/*) rpath="${path#${base}/}";;
        *)  # find common root directory between base and path
            common=; tmppath=${path}; newpath=${path}; newbase=${base}
            while test "${newpath%%/*}" = "${newbase%%/*}"; do
                case "${newbase}" in */*)newbase="${newbase#*/}";; *) newbase=;; esac
                case "${newpath}" in */*)newpath="${newpath#*/}";; *) newpath=;; esac
                common="${common}${tmppath%%/*}";
                case "${tmppath}" in */*) common="${common}/"; tmppath="${tmppath#*/}";; *) break;; esac
            done
            log 4 "+ relpath ${path} ${base}: COMMON = ${common} newpath: $newpath newbase: $newbase"
            bpref=
            while test -n "${newbase}"; do
                bpref="${bpref}${bpref:+/}.."
                case "${newbase}" in */*) newbase=${newbase#*/};; *) break;;esac; done
            rpath="${bpref}${bpref:+${newpath:+/}}${newpath}"
            ;;
    esac
    rpath="${rpath}${file:+/${file}}"
    echo "${rpath}"
    log 4 "--> relpath ${path} ${base} = ${rpath}"
}
is_binary() {
    local file=$1
    local lowfile=$(echo "${file}" | tr '[:upper:]' '[:lower:]')
    test -x "${file}" || return 1
    case "${lowfile}" in
        *.dylib|*.dylib.[0-9]*|*.so|*.so.[0-9]*|*.dll|*/library/frameworks/*.framework/*|*.pyd) return 1;;
    esac
    case "${build_sysname}" in darwin*) test -n "$(otool -D "${file}" | tail -n+2)" && return 1;; esac
    return 0
}
is_loader_rpath() {
    case "${exe_rpath}" in
        '@loader_path'*|'$ORIGIN'*) true;;
        *) false;;
    esac
}
get_rpath() {
    local _rpath=$1 _exe_rpath=$2 _exec_path=$3 _loader_path=${4:-$3} _new_rpath
    case "${_rpath}" in
        :/*|@*|\$*) echo "${_rpath#:}";;
        :*) _new_rpath="${_rpath#:}";;
        *) if is_loader_rpath "${_exe_rpath}"
           then _new_rpath="$(relpath "${_rpath}" "${_loader_path}")"
           else _new_rpath="$(relpath "${_rpath}" "${_exec_path}")"
           fi;;
    esac
    echo "${_exe_rpath}${_new_rpath:+/}${_new_rpath}"
}

for arg in "$@"; do
    case "${arg}" in
        -h) show_help 0;;
        -L*) libs_path=${arg#-L}; test -n "${libs_path}" || show_help 1; libs_path=$(rstrip_slash "${libs_path}");;
        -X*) arg=${arg#-X}; test -n "${arg}" || show_help 1; arg=$(rstrip_slash "${arg}")
             test -d "${arg}" && arg="${arg}/*"; excludes[${#excludes[@]}]=${arg};;
        -R*) arg=${arg#-R}; test -n "${arg}" || show_help 1;   rpaths[${#rpaths[@]}]=$(rstrip_slash "${arg}");;
        -E*) arg=${arg#-E}; test -n "${arg}" || show_help 1; _exec_path="$(abspath "${arg}")"
             test -f "${_exec_path}" && _exec_path=$(dirname "${_exec_path}")
             exec_paths[${#exec_paths[@]}]=${_exec_path};;
        -S)  dostrip=yes;;
        -V)  loglevel=$((loglevel+1));;
        -T)  do_tests=yes;;
        -*) echo "!! unknown option '${arg}'"; exit 1;;
        *) maintargets[${#maintargets[@]}]="${arg}";;
    esac
done

if test -z "${do_tests}" -a ${#exec_paths[@]} -eq 0 && { test ${#maintargets[@]} -eq 0 || ! is_binary "${maintargets[0]}"; }; then
    echo "!! error: -E option required unless first target '${maintargets[0]}': is a binary"; exit 1
fi

rpaths[0]=${libs_path}

ldd=ldd
ldd_get_libs() { #ldd "$@" | awk '/[^:]$/ { print $3 }'; }
    "${ldd}" "$@" | awk '// { sub(/^[[:space:]]*/, ""); } /not found/ { gsub(/[=>()[:space:]]/, "_"); print $0; } \
                         /=>/ { sub(/^[^>]*>[[:space:]]*/, ""); sub(/[[:space:]]*\(0x[0-9a-fA-F]*\).*/, ""); print $0; }'
}

case "${build_sysname}" in
    mingw*|cygwin*|msys*)
        get_file_rpath() { return 0; }
        del_file_rpaths() { return 0; }
        reset_file_rpaths() { return 0; }
        install_name_tool() { return 0; }
        if objdump -h "${maintargets[0]}" | grep -Eq "i386|i686" > /dev/null 2>&1; then
            ldd=$(which -a ntldd mingw-ldd ldd 2> /dev/null | head -n1)
        fi
        get_libs() { 
            local -a _lddargs; unset _lddargs
            local _d
            case "$(basename "${ldd}")" in mingw-ldd*)
                _lddargs[${#_lddargs[@]}]="--dll-lookup-dirs" 
                IFSbak=$IFS; IFS=";"; for _d in $(echo "${PATH}${LD_LIBRARY_PATH:+;}${LD_LIBRARY_PATH}" | sed -e 's/\([^\\]\):/\1;/g'); do
                    test -d "${_d}" && _lddargs[${#_lddargs[@]}]="${_d}"
                done; IFS=$IFSbak; _lddargs[${#_lddargs[@]}]="--";;
            esac
            ldd_get_libs "${_lddargs[@]}" "$@"
        };;
    linux*|*bsd*)
        exe_rpath='$ORIGIN'
        chrpath=$(which chrpath 2> /dev/null)
        get_file_rpath() {
            local target=$1
            "${chrpath}" "${target}" 2> /dev/null \
                | sed -ne 's/.*RUNPATH[[:space:]]*=[[:space:]]*\([^[:space:]]*\).*/\1/p'
        }
        del_file_rpaths() { # rpath cannot be set after that
            local target=$1; log 2 "+ deleting rpaths ($(get_file_rpath "${target}")) for ${target}"
            "${chrpath}" -d "${target}" > /dev/null
        }
        reset_file_rpaths() {
            local target=$1; log 2 "+ reseting rpaths ($(get_file_rpath "${target}")) for ${target}"
            "${chrpath}" -r "" "${target}" > /dev/null
        }
        install_name_tool() {
            log 3 ">> install_name_tool $@"
            case "$1" in
                -add_rpath|-rpath)
                    local rpath=$2 target=${4:-3}
                    rp_current_rpath=$(get_file_rpath "${target}")
                    #test $loglevel > 4 && echo "**ICONV current=$rp_current_rpath"
                    case ":${rp_current_rpath}:" in
                         *:"${rpath}":*) ;;
                         *) rp_current_rpath="${rp_current_rpath}${rp_current_rpath:+:}${rpath}";;
                    esac
                    #test $loglevel > 4 && echo "**ICONV new=$rp_current_rpath"
                    "${chrpath}" -r "${rp_current_rpath}" "${target}" > /dev/null;;
                -id) local libname=$2 target=$3;; # "${chrpath}" -r "${rp_current_rpath}" "${target}";;
                -change) local olddep=$2 newdep=$3 target=$4;;
            esac
        }
        get_libs() { ldd_get_libs "$@"; }
        ;;
    darwin*)
        install_name_tool() { log 3 ">> install_name_tool $@"; command install_name_tool "$@"; }
        get_file_rpath() {
            local target=$1
            otool -l "${target}" 2>/dev/null | grep -A3 RPATH \
                | sed -ne 's/^[[:space:]]*path[[:space:]][[:space:]]*\([^[:space:]]*\).*/\1/p' \
                | tr '\n' ':' | sed -e 's/:$//'
        }
        del_file_rpaths() {
            local target=$1 rp; IFSbak=$IFS; IFS=':'; for rp in $(get_file_rpath "${target}"); do
                log 2 "+ deleting rpath (${rp}) for ${target}"
                install_name_tool -delete_rpath "${rp}" "${target}" # 2>/dev/null
            done; IFS=${IFSbak}
        }
        reset_file_rpaths() { local target=$1; del_file_rpaths "${target}"; return $?; }
        get_libs() { otool -L "$@" | awk '/[^:]$/ { print $1 }'; }
        ;;
esac

###################################################
if test -n "${do_tests}"; then
    test_relpath() {
        test -t && { red="\x1b[01;31m"; green="\x1b[01;32m"; yellow="\x1b[00;33m"; rst="\x1b[00m"; } \
                || { red=;green=;yellow=;rst=; }
        local path=$1 base=$2
        printf -- "++ $path from $base --> "; rpath=$(relpath "${path}" "${base}")
        printf -- "${rpath} ... "
        if test -d "${path}" -a -d "${base}"; then
            newpath=$(cd "${path}" 2>/dev/null && pwd)
            (cd "${base}/${rpath}" 2>/dev/null && test "$(pwd)" = "${newpath}" && printf -- "${green}OK${rst}\n" || printf -- "${red}FAILED${rst}\n")
        else
            expected="$(abspath "$(stripslashes "${base}/${rpath}")")"
            test "${expected}" = "$(abspath "$(stripslashes "${path}")")" && printf -- "${green}OK${rst} " || printf -- "${red}FAILED${rst}(${expected}) "
            printf "${yellow}cannot check directory${rst}\n"
        fi
    }
    for arg in "/usr:/usr/share/man//" "${PWD}:/usr/share/man" "/usr/share/man:/usr/lib" \
               "${HOME}/.config/gtk-3.0:${HOME}/.local/share/applications" \
               "${HOME}/.config:${HOME}" "${HOME}:${PWD}" "${HOME}:${HOME}" \
               "/home/__someone__/data:///__FOLDER__///somewhere/else//" \
               "home/__someone__///data:__FOLDER__/somewhere/else///" \
               "/home/__someone__/something:/home/__someone/somethingelse/data" \
               "/home/__someone__/something:/home/__someone/something" \
               "__home__:__FOLDER__" "__home__:__FOLDER__/__other" "__home__/../__HOME2:__FOLDER__/././../__FOLDER2" \
               "__home folder__/sub dir1/sub dir 2:__home folder__/sub dir 3" \
    ; do
        base=${arg%:*}; path=${arg#*:}
        test_relpath "${path}" "${base}"
        test_relpath "${base}" "${path}"
    done
    exit $?
fi

###################################################
printf -- "\n**********************************************\n"
printf -- "* running $(basename "$0"), strip:${dostrip}\n"
printf -- "  + targets:\n"
for _target in "${maintargets[@]}"; do
    printf -- "    $(relpath "${_target}" "$(pwd)")\n";
done
test ${#exec_paths[@]} -gt 0 \
&& printf -- "  + other exec paths:\n" \
&& for _exec_path in "${exec_paths[@]}"; do
    printf -- "    $(relpath "${_exec_path}" "$(pwd)")\n";
   done
printf -- "  + main lib rpath:\n    $(relpath "${libs_path}" "$(pwd)")\n"
test ${#rpaths[@]} -gt 1 \
&& printf -- "  + other rpaths:\n" \
&& for _rpath in "${rpaths[@]}"; do
    printf -- "    $(relpath "${_rpath}" "$(pwd)")\n";
   done
test ${#excludes[@]} -gt 0 \
&& printf -- "  + excludes:\n" \
&& printf -- "    %s\n" "${excludes[@]}"
printf -- "\n"

##################################################################################
unset errors; declare -a errors
unset warnings; declare -a warnings
for maintarget in "${maintargets[@]}"; do
    unset targets; declare -a targets

    # Update the exec_path and loader_path, add the target with its loader to the target processing list
    maintarget_path=$(cd "$(dirname "${maintarget}")" && pwd)
    if is_binary "${maintarget}"; then
        exec_path=${maintarget_path}
        loader_path=${maintarget_path}
    else
        test -z "${exec_path}" && exec_path=${exec_paths[0]}
        loader_path=${exec_path}
    fi
    targets[${#targets[@]}]="${maintarget}:${loader_path}"

    # Process the target processing list
    i=0; while test $i -lt ${#targets[@]}; do target=${targets[$i]}; loader_path=${target#*:}; target=${target%:*}; i=$((i+1))
        target_name=$(basename "${target}")
        target_dir=$(dirname "${target}")
        lowtarget=$(echo "${target}" | tr "[:upper:]" "[:lower:]")
        short_target="$(relpath "${target_dir}" "$(pwd)")/${target_name}"
        echo "+ target [${short_target}]"

        test -f "${target}" || { echo "!! target '${target_name}' not found"; errors[${#errors[@]}]="[${target_name}] file not found!"; continue ; }

        target_libdir="${exec_path}/$(relpath "${libs_path}" "${exec_path}")"

        log 2 "+ [${short_target}] exec_path=$(relpath "${exec_path}" "$(pwd)") loader_path=$(relpath "${loader_path}" "$(pwd)")"

        _myrpath=; _is_binary=
        chmod 'u+w,u+r' "${target}"
        is_binary "${target}" && _is_binary=yes

        if test -z "${_is_binary}"; then
            install_name_tool -id "@rpath/${target_name}" "${target}" \
            || errors[${#errors[@]}]="[${target_name}] nametool id (${target_name})"
        fi

        if test -z "${_is_binary}" && ! is_loader_rpath "${_exe_rpath}"; then
            del_file_rpaths "${target}"
        else
            reset_file_rpaths "${target}"
            unset _origins; declare -a _origins
            if is_loader_rpath "${_exe_rpath}"; then
                #in $ORIGIN/@loader_path configuration we need to add a different loader_path when a lib is using a lib
                _myrpath=${target_dir}
                _origins[${#_origins[@]}]=${loader_path}
                _origins[${#_origins[@]}]=${target_dir}
            else
                _myrpath=
                _origins[${#_origins[@]}]=${exec_path}
            fi
            for _origin in "${_origins[@]}" "${exec_paths[@]}"; do
                for rpath in "${rpaths[@]}" ${_myrpath}; do
                    new_rpath=$(get_rpath "${rpath}" "${exe_rpath}" "${_origin}" "${_origin}")
                    install_name_tool -add_rpath "${new_rpath}" "${target}" 2> /dev/null \
                    || install_name_tool -rpath "${new_rpath}" "${new_rpath}" "${target}" \
                    || errors[${#errors[@]}]="[${target_name}] exe rpath add/update (${new_rpath})"
                done
            done
        fi
        test -z "${dostrip}" || { strip -S "${target}" && echo "+ $target stripped." || errors[${#errors[@]}]="strip $target"; }

        # Process the dependencies of current target, add them to target processing list if necessary.
        IFSbak=$IFS; IFS=$'\n'; for lib in `get_libs "${target}"`; do IFS=$IFSbak
          lib_name=$(basename "${lib}")
          lowlib=$(echo "${lib}" | tr "[:upper:]" "[:lower:]")

          case "${lowlib}" in
            /usr/lib/*|/usr/lib32/*|/usr/lib64/*) ;;
            /system/library/*) ;;
            /lib/*|/lib64/*|/lib32/*) ;;
            /c/windows/*|c:\\windows\\*|c:/windows/*) ;;
            @*) install_name_tool -change "${lib}" "@rpath/${lib_name}" "${target}" \
                    || errors[${#errors[@]}]="[${target_name}] nametool change @ (${lib_name})";;
            *)  copy=yes; for x in "${excludes[@]}"; do
                    eval "case '${lib}' in ${x}) copy=; warnings[${#warnings[@]}]='ignoring $lib (excludes)'; break;; esac"
                done
                for rp in "${rpaths[@]}"; do
                    testlib="${exec_path}/$(get_rpath "${rp}" "" "${exec_path}" "")/${lib_name}"
                    test -f "${testlib}" && { copy=; break; }
                done

                install_name_tool -change "${lib}" "@rpath/${lib_name}" "${target}" \
                    || errors[${#errors[@]}]="[${target_name}] nametool change * (${lib_name})"

                test -z "${copy}" \
                || if test -e "${lib}" && mkdir -p "${target_libdir}" && cp -pL "${lib}" "${target_libdir}" && echo "${lib} -> $(relpath "${target_libdir}" "$(pwd)")"; then
                    chmod 'u+w,u+r' "${target_libdir}/${lib_name}"
                    targets[${#targets[@]}]="${target_libdir}/${lib_name}:${target_dir}"
                else
                    errors[${#errors[@]}]="[${target_name}] cp '$lib'"
                fi
                ;;
          esac
        done
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

