

BIN_CM = checkModifierKeys
SRC_CM = checkModifierKeys.m
OBJ_CM = $(SRC_CM:.m=.o)

BIN_CK = checkKeys
SRC_CK = checkKeys.m
OBJ_CK = $(SRC_CK:.m=.o)

OSX_VERSION_MIN=
OSX_VERSION_MIN_ARG= -mmacosx-version-min=$(OSX_VERSION_MIN)
OSX_VERSION_MIN_ARG$(OSX_VERSION_MIN)=

ARCHS		= -arch i386 -arch x86_64
CFLAGS 		= $(ARCHS) $(OSX_VERSION_MIN_ARG)
OBJCFLAGS 	= $(CFLAGS)
CXXCFLAGS 	= $(CFLAGS)
LDFLAGS 	= $(ARCHS) -framework Carbon -framework ApplicationServices $(OSX_VERSION_MIN_ARG)
CC 		= clang
CXX		= clang++
OBJC 		= clang
RM		= rm -f

all: $(BIN_CM) $(BIN_CK)

$(BIN_CM): $(OBJ_CM) Makefile
	$(OBJC) $(LDFLAGS) -o $(BIN_CM) $(OBJ_CM)

$(BIN_CK): $(OBJ_CK) Makefile
	$(OBJC) $(LDFLAGS) -o $(BIN_CK) $(OBJ_CK)

clean:
	$(RM) $(OBJ_CM) $(OBJ_CK)

distclean: clean
	$(RM) $(BIN_CM) $(BIN_CK)

