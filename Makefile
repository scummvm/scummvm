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

CFLAGS   = -g -O -Wall -Wstrict-prototypes -Wuninitialized -Wno-long-long -Wno-multichar -Wno-unknown-pragmas -Werror
DEFINES  =
LDFLAGS :=
INCLUDES:= -I. -Icommon
LIBS	 =
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
	$(RM) $(ZIPFILE)
	$(ZIP) $(ZIPFILE) $(DISTFILES)

# Until we add a nice configure tool, default to the SDL build rules
build.rules:
	$(CP) backends/sdl/build.rules build.rules
