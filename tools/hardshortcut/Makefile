# Vega Strike
# Copyright (C) 2022 Vincent Sallaberry
#
# http://vegastrike.sourceforge.net/
# https://github.com/vsallaberry/privgold-pandora
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
# ***************************************************************************
# optional make ARGS:
#   OS=windows
#   ARCH="-arch x86_64"
#   OSX_VERSION_MIN=10.7
#   SHORTCUT_CPPFLAGS eg: SHORTCUT_CPPFLAGS="-DSHORTCUT_EXECPATH"
#   SHORTCUT_CFLAGS eg: SHORTCUT_CFLAGS=\{\\\"run\\\",\\\"bin/run\\\",NULL,\\\"setup\\\",\\\"bin/run\\\",\\\"--setup\\\",NULL,NULL\}
#   SHORTCUT_LDFLAGS

cmd_UNAME_SYS	= uname | tr '[A-Z]' '[a-z]' | sed -e 's/[^A-Za-z0-9]/_/g' -e 's/^mingw.*/windows/' -e 's/^cygwin.*/windows/' -e 's/^msys.*/windows/'
tmp_UNAME_SYS	!= $(cmd_UNAME_SYS)
tmp_UNAME_SYS	?= $(shell $(cmd_UNAME_SYS))
OS		:= $(tmp_UNAME_SYS)

BIN		= hardshortcut
SRC_windows	= Resources.rc
SRC		= hardshortcut.c $(SRC_$(OS))
OBJ1		= $(SRC:.c=.o)
OBJ		= $(OBJ1:.rc=.res)
RESOURCES_PATH	= ../../vegastrike

# add the -macosx-version-min only if argument OSX_VERSION_MIN in given
OSX_VERSION_MIN=
OSX_VERSION_MIN_ARG= -mmacosx-version-min=$(OSX_VERSION_MIN)
OSX_VERSION_MIN_ARG$(OSX_VERSION_MIN)=

# add the -D_GNU_SOURCE for all except windows
CPPFLAGS_COMMON_$(OS) = -D_GNU_SOURCE
CPPFLAGS_COMMON_windows	=
CPPFLAGS_COMMON	= $(CPPFLAGS_COMMON_$(OS))

ARCHS		=
CPPFLAGS	= $(SHORTCUT_CPPFLAGS) $(CPPFLAGS_COMMON_$(OS))
OPTIFLAGS	= -O3 -pipe $(ARCHS) $(OSX_VERSION_MIN_ARG)
COMMONFLAGS	= -Wall $(OPTIFLAGS)
CFLAGS 		= -std=c99 $(COMMONFLAGS) ${SHORTCUT_CFLAGS}
OBJCFLAGS 	= $(COMMONFLAGS)
CXXCFLAGS 	= $(COMMONFLAGS)
RCFLAGS		= -O coff -I$(RESOURCES_PATH)
LDFLAGS_windows	= -mwindows
LDFLAGS 	= $(OPTIFLAGS) $(LDFLAGS_$(OS)) $(SHORTCUT_LDFLAGS)
CC 		= clang
CXX		= clang++
OBJC 		= clang
WINDRES		= windres
RM		= rm -f
CP		= cp

all: $(BIN)

Resources.rc: $(RESOURCES_PATH)/Resources.rc
	$(CP) $< $@

$(BIN): $(OBJ) Makefile
	$(CC) $(LDFLAGS) -o $(BIN) $(OBJ)

clean:
	$(RM) $(OBJ)

distclean: clean
	$(RM) $(BIN) Resources.rc

$(OBJ): Makefile

#.SUFFIXES: .rc .res
#.rc.res:
#	$(WINDRES) $(CPPFLAGS) $(RCFLAGS) $< $@

# run manually pre-processor because mingw32 i686 uses gcc -E and messes up quotes (clang64 x86_64 is ok)
.SUFFIXES: .rc .res
.rc.res:
	"$(WINDRES)" $(CPPFLAGS) $(RCFLAGS) "$<" "$@" \
	|| { echo "+ windres fallback..."; "$(CC)" -E -x c -DRC_INVOKED $(CPPFLAGS) "$<" -o "$(<).pre" \
	     && "$(WINDRES)" -v $(RCFLAGS) --preprocessor "$(CC)" --preprocessor-arg -E \
	        --preprocessor-arg -xc --preprocessor-arg -DRC_INVOKED "$(<).pre" "$@" \
	     && rm "$(<).pre"; }

