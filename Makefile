# $Header$

CC	= gcc
CFLAGS	= -g -Wno-multichar
DEFINES	= -DUNIX -DHAVE_READLINE -DUSE_TIMIDITY
LDFLAGS := 
INCLUDES:= `sdl-config --cflags`
CPPFLAGS= $(DEFINES) $(INCLUDES)
LIBS	= `sdl-config --libs` -lreadline -lncurses -lhistory
ZIPFILE := scummvm-`date '+%Y-%m-%d'`.zip

INCS	= scumm.h scummsys.h stdafx.h

OBJS	= actor.o boxes.o costume.o gfx.o object.o resource.o \
	  saveload.o script.o scummvm.o sound.o string.o \
	  sys.o verbs.o sdl.o script_v1.o script_v2.o debug.o gui.o \
	  imuse.o debugrl.o

DISTFILES=$(OBJS:.o=.cpp) Makefile scumm.h scummsys.h stdafx.h stdafx.cpp \
	  windows.cpp debugrl.h whatsnew.txt readme.txt copying.txt \
	  scummvm.dsp scummvm.dsw 

.cpp.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $(<)

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
