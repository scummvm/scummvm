CC	= gcc
CFLAGS	= -g -O2 -Wno-multichar
DEFINES	= -DUNIX -DSCUMM_BIG_ENDIAN -DSCUMM_NEED_ALIGNMENT
LDFLAGS = `sdl-config --libs`
INCLUDES= `sdl-config --cflags`
CPPFLAGS= $(DEFINES) $(INCLUDES)

INCS	= scumm.h scummsys.h stdafx.h

OBJS	= actor.o boxes.o costume.o gfx.o object.o resource.o \
	  saveload.o script.o scummvm.o sound.o string.o \
	  sys.o verbs.o sdl.o
	
.cpp.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $(<)

all: scummvm

scummvm: $(OBJS)
	$(CC) $(LDFLAGS) -o $(@) $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS) scummvm

check:
$(OBJS): $(INCS)
