vpath %.cpp ../:../sound/:../v3/:../v4/:../simon/
vpath %.h   ../

CC      = g++
CFLAGS  = -Wno-multichar -fstrength-reduce -O2
DEFINES = -DNO_PPCINLINE_STDARG -DNO_PPCINLINE_VARARGS
LDFLAGS := -noixemul -s
INCLUDES:= -I../ -I../sound
CPPFLAGS= $(DEFINES) $(INCLUDES)
LIBS	= -lamiga -lamigastubs -lcdda
ZIPFILE := scummvm-`date '+%Y-%m-%d'`.zip

INCS	= scumm.h scummsys.h stdafx.h

OBJS	= actor.o akos.o boxes.o config-file.o costume.o gfx.o object.o resource.o \
	saveload.o script.o scummvm.o sound.o string.o sys.o verbs.o \
	morphos.o morphos_sound.o morphos_start.o script_v1.o script_v2.o debug.o gui.o \
	imuse.o fmopl.o mixer.o mididrv.o debugrl.o vars.o insane.o \
	gameDetector.o init.o resource_v3.o resource_v4.o main.o \
	midi.o simon.o simondebug.o simonitems.o simonres.o simonsys.o simonverb.o simonvga.o

DISTFILES=$(OBJS:.o=.cpp) Makefile scumm.h scummsys.h stdafx.h stdafx.cpp \
	windows.cpp debugrl.h whatsnew.txt readme.txt copying.txt \
	scummvm.dsp scummvm.dsw sound/fmopl.h

.cpp.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $(<) -o $*.o

all: scummvm

scummvm: $(OBJS)
	$(CC) $(LDFLAGS) -o $(@) $(OBJS) $(LIBS)

#$(OBJS): Makefile

clean:
	rm -f $(OBJS) scummvm

dist:
	rm -f $(ZIPFILE)
	zip -q $(ZIPFILE) $(DISTFILES)

check:
$(OBJS): $(INCS)
