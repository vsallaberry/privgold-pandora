#!/bin/sh
mydir=$(cd `dirname "$0"`; pwd)
curdir=$(pwd)

version_file=Version.txt
setup_config_file=setup.config
configname_key=config_file
engine_key=GL-Renderer
setup_key=SetupProg
log_key=Logging
vega_oldhome="${HOME}/.privgold100"

sed=sed
tr=tr
mkdir="mkdir -p"
cp="cp -v"
diff="diff -u -q"
os=$(uname -s | tr "[:upper:]" "[:lower:]")
arch=$(uname -m)
do_setup=
bin_path=

while test -n "$1"; do
    case "$1" in
        --)      shift; break ;;
        --setup) do_setup=yes;;
        --bin=*) bin_path=${1#--bin=};;
        -*) echo "Usage: $(basename "$0") [--help] [--setup] [--bin=<program-to-run>] [-- [<program-options>]]"
            case "$1" in -h|--help) exit 0;; *) exit 1; esac;;
    esac
    shift
done

#
# Go in Data directory
#
data_dir="${mydir}/../Resources/data"
test -d "${data_dir}" || data_dir="${mydir}/../data"
cd "${data_dir}"

#
# extract vegastrike setup config file from data/setup.config
#
config_name=$("$sed" -n -e "s/^[[:space:]]*${configname_key}[[:space:]][[:space:]]*\(.*\)/\1/p" "${setup_config_file}" | "$tr" '\r' '\n')
test -z "${config_name}" && config_name="vegastrike.config"

#
# extract vegastrike user config folder from vegastrike data Version.txt
#
vega_name=$("$sed" -n -e "s/^\([^#]*\)$/\1/p" "${version_file}" | "$tr" '\r' '\n')
test -z "${vega_name}" && vega_name=".vegastrike"
vega_home="${HOME}/${vega_name}"

#
# get vegastrike config file
#
config="${vega_home}/${config_name}"

#
# create/update config if not existing
#
test -d "${vega_home}" || {
    ${mkdir} "${vega_home}"
    test -d "${vega_oldhome}" && ${cp} -a "${vega_oldhome}/save" "${vega_oldhome}/serialized_xml" "${vega_home}/"
}
test -f "${config}" || ${cp} "${config_name}" "${config}"
test -d "${vega_home}/save" || ${mkdir} "${vega_home}/save"
test -d "${vega_home}/serialized_xml/New_Game" || ${mkdir} "${vega_home}/serialized_xml/New_Game"
${diff} "Version.txt" "${vega_home}/Version.txt" >/dev/null 2>&1 || ${cp} Version.txt "${vega_home}"
${diff} "${setup_config_file}" "${vega_home}/${setup_config_file}" >/dev/null 2>&1 || ${cp} "${setup_config_file}" "${vega_home}"
${diff} "New_Game" "${vega_home}/save/New_Game" >/dev/null 2>&1 || ${cp} New_Game "${vega_home}/save"
${diff} "${vega_name}/serialized_xml/New_Game/tarsus.begin.csv" \
        "${vega_home}/serialized_xml/New_Game/tarsus.begin.csv"  >/dev/null 2>&1 \
    || ${cp} "${vega_name}/serialized_xml/New_Game/tarsus.begin.csv" \
             "${vega_home}/serialized_xml/New_Game/tarsus.begin.csv"
test -f "${vega_home}/save.4.x.txt" || { echo New_Game > "${vega_home}/save.4.x.txt.new"; \
                                         ${cp} "${vega_home}/save.4.x.txt.new" "${vega_home}/save.4.x.txt"; }

#
# look for graphics engine,setup program, and log config to be used in config file
#
engine=$("$sed" -n -e "s/^[[:space:]]*#[[:space:]]*set[[:space:]][[:space:]]*${engine_key}[[:space:]][[:space:]]*\([^\n\r]*\)/\1/p" \
                "${config}" 2>/dev/null | "$tr" '\r' '\n')

setups=$("$sed" -n -e "s/^[[:space:]]*#[[:space:]]*set[[:space:]][[:space:]]*${setup_key}[[:space:]][[:space:]]*\([^\n\r]*\)/\1/p" \
                "${config}" 2>/dev/null | "$tr" '\r' '\n')

logs=$("$sed" -n -e "s/^[[:space:]]*#[[:space:]]*set[[:space:]][[:space:]]*${log_key}[[:space:]][[:space:]]*\([^\n\r]*\)/\1/p" \
                "${config}" 2>/dev/null | "$tr" '\r' '\n')

case "${engine}" in
    SDL1|SDL2|GLUT);;
    *) engine=SDL2;;
esac
case "${setups}" in
    "vssetup_dlg")  setups="${setups} vssetup";;
    "vssetup")      setups="${setups} vssetup_dlg";;
    *)              setups="vssetup vssetup_dlg";;
esac
case "${logs}" in
    "log_none")     log_file=/dev/null; setuplog_file=${log_file};;
    "log_print")    log_file=/dev/stderr; setuplog_file=${log_file};;
    "log_file")     log_file="${vega_home}/vegastrike.log"; setuplog_file="${vega_home}/vssetup.log";;
    *)              log_file=/dev/stderr; setuplog_file=${log_file};;
esac
( umask 0022; touch "${log_file}"; touch "${setuplog_file}" ) >/dev/null 2>&1
test -w "${log_file}" || log_file=/dev/stderr

# ensure the home config is newer than data one, else setup will pickup data config
#not needed: it is good that setup takes the new one
#test "${data_dir}/${config_name}" -nt "${config}" && touch "${config}"

#
# some x11 backward compatibility features
#
if test -z "${DISPLAY}"; then
    setenv DISPLAY :0.0 2>/dev/null || true
    export DISPLAY=:0.0 2>/dev/null || true
fi
if test -z "${XAUTHORITY}"; then
    setenv XAUTHORITY ~/.Xauthority 2>/dev/null || true
    export XAUTHORITY=~/.Xauthority 2>/dev/null || true
fi

cd "${curdir}"
echo
echo "-----------------------------------------------------"
echo "Vegastrike launcher"
echo "  os        ${os} ${arch}"
echo "  home      ${vega_home}"
echo "  config    ${config}"
echo "  gfx       ${engine}"
echo "  setups    ${setups}"
echo "  log       ${log_file}"
echo "-----------------------------------------------------"
echo

#
# run setup if ALT key is pressed
#
case "${os}" in
    darwin*) export TERMINFO_DIRS="${mydir}/../Resources/share/terminfo";;
    linux*|*bsd*)
        #export LD_LIBRARY_PATH="${mydir}/../lib:${mydir}/../lib/gtk:${mydir}/../lib/misc${LD_LIBRARY_PATH:+:}${LD_LIBRARY_PATH}"
        if ! amixer 2>&1 > /dev/null; then
            echo "*** using alsa-lib from privateer package"
            export ALSA_CONFIG_DIR="${mydir}/../etc/alsa"
            export LD_LIBRARY_PATH="${mydir}/../lib/alsa-lib${LD_LIBRARY_PATH:+:}${LD_LIBRARY_PATH}"
        fi
        export LD_LIBRARY_PATH="${mydir}/../lib${LD_LIBRARY_PATH:+:}${LD_LIBRARY_PATH}"
        test -n "${do_setup}" -o -n "${bin_path}" && export LD_LIBRARY_PATH="${mydir}/../lib/gtk${LD_LIBRARY_PATH:+:}${LD_LIBRARY_PATH}"
        test -n "${bin_path}" && export LD_LIBRARY_PATH="${mydir}/../lib/misc${LD_LIBRARY_PATH:+:}${LD_LIBRARY_PATH}"
        export TERMINFO_DIRS="${mydir}/../share/terminfo"
        ;;
esac

if test -n "${do_setup}" -o "$("${mydir}/checkModifierKeys" option 2>/dev/null)" = "1"; then

    # Run SETUP
    cd "${data_dir}"

    for setup in ${setups}; do
        test -x "${mydir}/${setup}" || continue
        if test "${setup}" = "vssetup_dlg"; then
            #This is not pretty but this bypasses sandboxing issues when launching another app
            #open -a Terminal.app -n "${mydir}/${setup}" && break
            case "${os}" in
                darwin*)
                    for f in /System/Applications/Utilities /Applications/Utilities /System/Applications /Applications; do
                        if test -x "${f}/Terminal.app/Contents/MacOS/Terminal"; then
                            "${f}/Terminal.app/Contents/MacOS/Terminal" "${mydir}/${setup}" & termpid=$! || continue
                            sleep 3
                            setuppid=$(pgrep -u$(id -u) -a "${setup}")
                            echo "Term pid=$termpid, Setup pid=$setuppid"
                            while ps -p${setuppid} >/dev/null 2>&1; do sleep 1; done
                            kill $termpid
                            break
                        fi
                    done
                    ;;
                linux*|*bsd*)
                    for term in $(which -a xterm gnome-terminal konsole "${TERM}" 2>/dev/null); do
                        if test -x "${term}"; then
                            exec "${term}" -e "${mydir}/${setup}" "$@"
                            break # break not neeed with exec
                        fi
                    done
                    ;;
            esac
        else
            exec "${mydir}/${setup}" "$@"
        fi && break
    done

elif test -n "${bin_path}"; then

    exec "${bin_path}" "$@"

else
    # Run VEGASTRIKE
    cd "${data_dir}"

    exec "${mydir}/vegastrike.${engine}" "$@"
fi

