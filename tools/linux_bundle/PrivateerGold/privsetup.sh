#!/bin/sh
mydir=$(cd "$(dirname "$0")"; pwd)
bindir=bin
exec "${mydir}/${bindir}/launcher.sh" --setup "$@"

