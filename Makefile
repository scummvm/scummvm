# $Header$

CC      = gcc
CFLAGS  = -g -O -Wall -Wstrict-prototypes -Wuninitialized -Wno-long-long -Wno-multichar 
DEFINES = -DUNIX
LDFLAGS :=
INCLUDES:= `sdl-config --cflags` -I./ -I./sound
CPPFLAGS= $(DEFINES) $(INCLUDES)

# Add -lmad for -DCOMPRESSED_SOUND_FILE
LIBS	= `sdl-config --libs` -lncurses

OBJS	= sdl.o

include Makefile.common
