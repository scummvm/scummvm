# $Header$

CXX     := c++
AR      := ar cru
RANLIB  := ranlib
RM      := rm -f
MKDIR   := mkdir -p
ECHO    := echo -n
CAT     := cat
RM      := rm -f
# recursive version of RM
RM_REC  := $(RM) -r
ZIP     := zip -q
CP      := cp

#######################################################################
# Default compilation parameters. Normally don't edit these           #
#######################################################################

CXXFLAGS:= -g -O
DEFINES :=
LDFLAGS :=
INCLUDES:= -I. -Icommon
LIBS	:=
OBJS	:=

# Turn on useful warnings
CXXFLAGS+= -Wall -pedantic -Wpointer-arith -Wcast-qual -Wcast-align -Wconversion
CXXFLAGS+= -Wshadow -Wstrict-prototypes -Wuninitialized -Wimplicit -Wundef
CXXFLAGS+= -Wno-long-long -Wno-multichar -Wno-unknown-pragmas -Wno-reorder
CXXFLAGS+= -Wwrite-strings -fcheck-new -Wctor-dtor-privacy -Wnon-virtual-dtor

# Seems GCC 2.95 doesn't support the following, so we do not yet turn them
# on by default (but I'd strongly recommend to all GCC 2.95 users to update
# to a better compiler like GCC 3.x).
# CXXFLAGS+= -ansi -W -Wno-unused-parameter -Woverloaded-virtual -Wdisabled-optimization -Wfloat-equal

# The following causes problems on some systems where the system header
# contain duplicate declarations already. That's really a bug in the 
# system headers, but since it causes lots of warnings on those systems,
# we don't enable it by default
#CXXFLAGS+= -Wredundant-decls

# Load the build rules & settings for the chosen backend
-include build.rules

#######################################################################
# Compile options - you can modify these to tweak ScummVM compilation #
#######################################################################

# Uncomment this to activate the MAD lib for compressed sound files
DEFINES += -DUSE_MAD
LIBS    += -lmad

# Uncomment this to activate the Ogg Vorbis lib for compressed sound files
# DEFINES += -DUSE_VORBIS
# LIBS    += -lvorbisfile -lvorbis

# Uncomment this to activate the ALSA lib for midi
# DEFINES += -DUSE_ALSA
# LIBS    += -lasound

# Uncomment this to cause warnings to be treated as errors
# CXXFLAGS+= -Werror


#######################################################################
# Control which modules are built - uncomment any to disable module   #
#######################################################################

# DISABLE_SCUMM = 1
# DISABLE_SIMON = 1
# DISABLE_SKY = 1
DISABLE_BS2 = 1


#######################################################################
# Misc stuff - you should normally never have to edit this            #
#######################################################################

include Makefile.common

dist:
	$(RM) $(ZIPFILE)
	$(ZIP) $(ZIPFILE) $(DISTFILES)

# Until we add a nice configure tool, default to the SDL build rules
build.rules:
	$(CP) backends/sdl/build.rules build.rules

deb:
	ln -sf dists/debian;
	debian/prepare
	fakeroot debian/rules binary

# Special target to create a application wrapper for Mac OS X
bundle_name = ScummVM.app
bundle: scummvm-static
	mkdir -p $(bundle_name)/Contents/MacOS
	mkdir -p $(bundle_name)/Contents/Resources
	echo "APPL????" > $(bundle_name)/Contents/PkgInfo
	cp Info.plist $(bundle_name)/Contents/
	cp scummvm.icns $(bundle_name)/Contents/Resources/
	cp scummvm-static $(bundle_name)/Contents/MacOS/scummvm
	strip $(bundle_name)/Contents/MacOS/scummvm

# Special target to create a static linked binary for Mac OS X
scummvm-static: $(OBJS)
	$(CXX) $(LDFLAGS) -o scummvm-static $(OBJS) \
		/sw/lib/libSDLmain.a /sw/lib/libSDL.a \
		/sw/lib/libmad.a \
		/sw/lib/libvorbisfile.a /sw/lib/libvorbis.a /sw/lib/libogg.a \
		-framework Cocoa -framework Carbon -framework IOKit \
		-framework OpenGL -framework AGL -framework QuickTime \
		-framework AudioUnit -framework AudioToolbox

.PHONY: deb bundle
