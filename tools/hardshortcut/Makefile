# optional make ARGS: 
#   OS=WIN32
#   OSX_VERSION_MIN=10.7
#   SHORTCUT_CPPFLAGS
#   SHORTCUT_CFLAGS
#   SHORTCUT_LDFLAGS

BIN		= hardshortcut
SRC_WIN32	= ../../vegastrike/Resources.rc
SRC		= hardshortcut.c $(SRC_$(OS))
OBJ1		= $(SRC:.c=.o)
OBJ		= $(OBJ1:.rc=.res)

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
RCFLAGS		= -O coff
LDFLAGS_WIN32	= -mwindows
LDFLAGS 	= $(OPTIFLAGS) $(LDFLAGS_$(OS)) $(SHORTCUT_LDFLAGS)
CC 		= clang
CXX		= clang++
OBJC 		= clang
WINDRES		= windres
RM		= rm -f

all: $(BIN)

$(BIN): $(OBJ) Makefile
	$(CC) $(LDFLAGS) -o $(BIN) $(OBJ)

clean:
	$(RM) $(OBJ)

distclean: clean
	$(RM) $(BIN)

$(OBJ): Makefile

.SUFFIXES: .rc .res
.rc.res:
	$(WINDRES) $(CPPFLAGS) $(RCFLAGS) $< $@

