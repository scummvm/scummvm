# $Header$

CC       = g++
CFLAGS   = -g -O -Wall -Wstrict-prototypes -Wuninitialized -Wno-long-long -Wno-multichar
DEFINES  =
LDFLAGS :=
INCLUDES:= -I./ -I./sound
LIBS	 = -lncurses

# Uncomment this to activate the MAD lib for compressed sound files
# DEFINES += -DCOMPRESSED_SOUND_FILE
# LIBS    += -lmad

# Uncomment this to activate the ALSA lib for midi
# DEFINES += -DUSE_ALSA
# LIBS    += -lasound


# Now, please choose a graphical output system between SDL and X11.
# Beware, only define one of them, otherwise the compilation will blow up.

# Comment this if you want to disable SDL output
OBJS	 = sdl.o
INCLUDE += `sdl-config --cflags`
LIBS    += `sdl-config --libs`
DEFINES += -DUNIX

# Uncomment this if you rather want X11 output
# OBJS     = x11.o
# DEFINES += -DUNIX_X11
# LDFLAGS := -L/usr/X11R6/lib -L/usr/local/lib
# INCLUDES+= -I/usr/X11R6/include
# LIBS    += -lpthread -lXext -lX11


CPPFLAGS= $(DEFINES) $(INCLUDES)

include Makefile.common

dist:
	rm -f $(ZIPFILE)
	zip -q $(ZIPFILE) $(DISTFILES)
