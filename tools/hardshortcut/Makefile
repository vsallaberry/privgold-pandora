# optional make ARGS: 
#   OS=windows
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

OSX_VERSION_MIN=
OSX_VERSION_MIN_ARG= -mmacosx-version-min=$(OSX_VERSION_MIN)
OSX_VERSION_MIN_ARG$(OSX_VERSION_MIN)=

ARCHS		= 
CPPFLAGS	= $(SHORTCUT_CPPFLAGS)
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

.SUFFIXES: .rc .res
.rc.res:
	$(WINDRES) $(CPPFLAGS) $(RCFLAGS) $< $@

