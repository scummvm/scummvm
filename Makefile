# $Header$

CXX      = c++
AR        = ar cru
RANLIB = ranlib
RM      = rm -f
MKDIR = mkdir -p
ECHO  = echo -n
CAT    = cat
RM      = rm -f
# recursive version of RM
RM_REC = $(RM) -r
ZIP = zip -q
CP  = cp

#######################################################################
# Default compilation parameters. Normally don't edit these           #
#######################################################################

CFLAGS  := -g -O -Wall -Wstrict-prototypes -Wuninitialized -Wno-long-long -Wno-multichar -Wno-unknown-pragmas
DEFINES :=
LDFLAGS :=
INCLUDES:= -I. -Icommon
LIBS	:=
OBJS	:=

# Load the build rules & settings for the chosen backend
-include build.rules

#######################################################################
# Compile options - you can modify these to tweak ScummVM compilation #
#######################################################################

# Enable this if you want ScummVM to dump all scripts it runs.
# This is mainly interesting for developers.
# DEFINES += -DDUMP_SCRIPTS

# Uncomment this to activate the MAD lib for compressed sound files
DEFINES += -DUSE_MAD
LIBS    += -lmad

# Uncomment this to activate the Ogg Vorbis lib for compressed sound files
# DEFINES += -DUSE_VORBIS
# LIBS    += -lvorbisfile -lvorbis

# Uncomment this to activate the ALSA lib for midi
# DEFINES += -DUSE_ALSA
# LIBS    += -lasound

# Uncomment this for stricter compile time code verification
# CFLAGS  += -Wshadow -Werror


#######################################################################
# Misc stuff - you should normally never have to edit this            #
#######################################################################

# Concat DEFINES and INCLUDES to for the CPPFLAGS
CPPFLAGS:= $(DEFINES) $(INCLUDES)

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
	$(CXX) $(LDFLAGS) -o scummvm-static $(OBJS) /sw/lib/libSDLmain.a /sw/lib/libSDL.a /sw/lib/libmad.a -framework OpenGL -framework AGL -framework IOKit -framework Cocoa -framework Carbon -framework QuickTime -framework AudioUnit

.PHONY: deb bundle
