vpath %.cpp ../:../sound/:../v3/:../v4/:../simon/:../gui/
vpath %.h   ../

CC      = g++
CFLAGS  = -Wno-multichar -fstrength-reduce -fno-rtti -O2
DEFINES = -DNO_PPCINLINE_STDARG -DNO_PPCINLINE_VARARGS
LDFLAGS := -noixemul -s
INCLUDES:= -I../ -I../sound
CPPFLAGS= $(DEFINES) $(INCLUDES)
LIBS	= -lamiga -lamigastubs -lcdda
ZIPFILE := scummvm-`date '+%Y-%m-%d'`.zip

INCS	= scumm.h scummsys.h stdafx.h

GUIOBJS = widget.o dialog.o newgui.o ListWidget.o ScrollBarWidget.o

SCUMMOBJS = actor.o akos.o boxes.o bundle.o costume.o debug.o debugrl.o gfx.o \
	object.o saveload.o script.o scummvm.o sound.o string.o sys.o vars.o verbs.o \
	script_v1.o script_v2.o gui.o imuse.o insane.o resource.o resource_v3.o resource_v4.o

SIMONOBJS =	midi.o simon.o simondebug.o simonitems.o simonres.o simonsys.o simonverb.o simonvga.o

OBJS	= config-file.o engine.o gameDetector.o fmopl.o mixer.o mididrv.o util.o main.o \
	morphos.o morphos_scaler.o morphos_sound.o morphos_start.o morphos_timer.o \
	$(GUIOBJS) $(SCUMMOBJS) $(SIMONOBJS)

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
