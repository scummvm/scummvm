CC	= gcc
CFLAGS	= -g -O2 -Wno-multichar
DEFINES	= -DUNIX
LDFLAGS = `sdl-config --libs`
INCLUDES= `sdl-config --cflags`
CPPFLAGS= $(DEFINES) $(INCLUDES)

INCS	= scumm.h scummsys.h stdafx.h

OBJS	= actor.o boxes.o costume.o gfx.o object.o resource.o \
	  saveload.o script.o scummvm.o sound.o string.o \
	  sys.o verbs.o sdl.o script_v7.o script_v1.o script_v2.o debug.o
	
.cpp.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $(<)

all: scummvm

scummvm: $(OBJS)
	$(CC) $(LDFLAGS) -o $(@) $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS) scummvm

check:
$(OBJS): $(INCS)
