# $Header$

CC       = g++
CFLAGS   = -g -O -Wall -Wstrict-prototypes -Wuninitialized -Wno-long-long -Wno-multichar
DEFINES  =
LDFLAGS :=
INCLUDES:= -I./ -I./sound
LIBS	 = -lncurses

# Uncomment this to activate the MAD lib for compressed sound files
#DEFINES += -DCOMPRESSED_SOUND_FILE
#LIBS    += -lmad

# Uncomment this to activate the ALSA lib for midi
# DEFINES += -DUSE_ALSA
# LIBS    += -lasound

# Now, please choose a graphical output system between SDL, SDL/GL and X11.
# Beware, only define one of them, otherwise the compilation will blow up.

# Comment this if you want to disable SDL output
OBJS	 = sdl.o
INCLUDES += `sdl-config --cflags`
LIBS    += `sdl-config --libs`
DEFINES += -DUNIX

# Uncomment this (instead of the above) to activate the SDL with OpenGL output
#OBJS	 = sdl_gl.o
#INCLUDES += `sdl-config --cflags`
#LIBS    += `sdl-config --libs` -lGL
#DEFINES += -DUNIX

# Uncomment this in addition to the above if you compile on Mac OS X
# LIBS	+= -framework QuickTime -framework AudioUnit
# DEFINES += -DMACOSX

# Uncomment this if you rather want X11 output
# OBJS     = x11.o
# DEFINES += -DUNIX_X11
# LDFLAGS := -L/usr/X11R6/lib -L/usr/local/lib
# INCLUDES+= -I/usr/X11R6/include
# LIBS    += -lpthread -lXext -lX11

# Enable this if you want ScummVM to dump all scripts it runs.
# This is mainly interesting for developers.
# DEFINES += -DDUMP_SCRIPTS

CPPFLAGS= $(DEFINES) $(INCLUDES)

include Makefile.common

dist:
	rm -f $(ZIPFILE)
	zip -q $(ZIPFILE) $(DISTFILES)
