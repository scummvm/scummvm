# $Header$

CC      = gcc
CFLAGS  = -g -O -Wall -Wstrict-prototypes -Wuninitialized -Wno-long-long -Wno-multichar 
DEFINES = -DUNIX
LDFLAGS :=
INCLUDES:= `sdl-config --cflags` -I./ -I./sound
CPPFLAGS= $(DEFINES) $(INCLUDES)

# Add -lmad for -DCOMPRESSED_SOUND_FILE
LIBS	= `sdl-config --libs` -lncurses -lm
ZIPFILE := scummvm-`date '+%Y-%m-%d'`.zip

INCS	= scumm.h scummsys.h stdafx.h

OBJS	= actor.o boxes.o costume.o gfx.o object.o resource.o \
	saveload.o script.o scummvm.o sound.o string.o \
	sys.o verbs.o sdl.o script_v1.o script_v2.o debug.o gui.o \
	sound/imuse.o sound/fmopl.o sound/mixer.o debugrl.o \
        akos.o vars.o insane.o gameDetector.o init.o \
	v3/resource_v3.o v4/resource_v4.o 2xsai.o main.o \
	simon/midi.o simon/simon.o simon/simonsys.o sound/mididrv.o

DISTFILES=$(OBJS:.o=.cpp) Makefile scumm.h scummsys.h stdafx.h stdafx.cpp \
	windows.cpp debugrl.h whatsnew.txt readme.txt copying.txt \
	scummvm.dsp scummvm.dsw sound/fmopl.h gui.h sound.h

.cpp.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $(<) -o $*.o

all: scummvm

scummvm: $(OBJS)
	$(CC) $(LDFLAGS) -o $(@) $(OBJS) $(LIBS)

$(OBJS): Makefile

clean:
	rm -f $(OBJS) scummvm

dist:
	rm -f $(ZIPFILE)
	zip -q $(ZIPFILE) $(DISTFILES)

check:
$(OBJS): $(INCS)
