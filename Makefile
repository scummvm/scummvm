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

CFLAGS   = -g -O -Wall -Wstrict-prototypes -Wuninitialized -Wno-long-long -Wno-multichar -Wno-unknown-pragmas
DEFINES  =
LDFLAGS :=
INCLUDES:= -I. -Icommon
LIBS	 =
OBJS	 =

# Load the build rules & settings for the chosen backend
-include build.rules

#######################################################################
# Compile options - you can modify these to tweak ScummVM compilation #
#######################################################################

# Enable this if you want ScummVM to dump all scripts it runs.
# This is mainly interesting for developers.
# DEFINES += -DDUMP_SCRIPTS

# Uncomment this to activate the MAD lib for compressed sound files
DEFINES += -DCOMPRESSED_SOUND_FILE
LIBS    += -lmad

# Uncomment this to activate the ALSA lib for midi
# DEFINES += -DUSE_ALSA
# LIBS    += -lasound

# Uncomment this to activate extended debugging support in Simon
DEFINES += -DSIMONDEBUG

# Uncomment this for stricter compile time code verification
# CFLAGS  += -Wshadow -Werror


#######################################################################
# Misc stuff - you should normally never have to edit this            #
#######################################################################

# Concat DEFINES and INCLUDES to for the CPPFLAGS
CPPFLAGS= $(DEFINES) $(INCLUDES)

include Makefile.common

dist:
	$(RM) $(ZIPFILE)
	$(ZIP) $(ZIPFILE) $(DISTFILES)

# Until we add a nice configure tool, default to the SDL build rules
build.rules:
	$(CP) backends/sdl/build.rules build.rules
