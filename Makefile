# $Header$

CXX      = c++
CFLAGS   = -g -O -Wall -Wstrict-prototypes -Wuninitialized -Wno-long-long -Wno-multichar
DEFINES  =
LDFLAGS :=
INCLUDES:= -I. -Icommon -Iscumm
LIBS	 = -lncurses
OBJS	 =

# Load the build rules & settings for the chosen backend
-include build.rules

# Enable this if you want ScummVM to dump all scripts it runs.
# This is mainly interesting for developers.
# DEFINES += -DDUMP_SCRIPTS

# Uncomment this to activate the MAD lib for compressed sound files
DEFINES += -DCOMPRESSED_SOUND_FILE
LIBS    += -lmad

# Uncomment this to activate the ALSA lib for midi
# DEFINES += -DUSE_ALSA
# LIBS    += -lasound

# Concat DEFINES and INCLUDES to for the CPPFLAGS
CPPFLAGS= $(DEFINES) $(INCLUDES)

include Makefile.common

dist:
	rm -f $(ZIPFILE)
	zip -q $(ZIPFILE) $(DISTFILES)

# Until we add a nice configure tool, default to the SDL build rules
build.rules:
	ln -s backends/sdl/build.rules
