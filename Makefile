CC	= gcc
CFLAGS	= -g -Wno-multichar
DEFINES	= -DUNIX
LDFLAGS := `sdl-config --libs`
INCLUDES:= `sdl-config --cflags`
CPPFLAGS= $(DEFINES) $(INCLUDES)
ZIPFILE := scummvm-`date '+%Y-%m-%d'`.zip

INCS	= scumm.h scummsys.h stdafx.h

OBJS	= actor.o boxes.o costume.o gfx.o object.o resource.o \
	  saveload.o script.o scummvm.o sound.o string.o \
	  sys.o verbs.o sdl.o

DISTFILES=actor.cpp boxes.cpp costume.cpp gfx.cpp object.cpp resource.cpp \
	  saveload.cpp script.cpp scummvm.cpp sound.cpp string.cpp \
	  sys.cpp verbs.cpp sdl.cpp \
	  Makefile scumm.h scummsys.h stdafx.h stdafx.cpp windows.cpp \
          whatsnew.txt readme.txt copying.txt scummvm.dsp scummvm.dsw 

.cpp.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $(<)

all: scummvm

scummvm: $(OBJS)
	$(CC) $(LDFLAGS) -o $(@) $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS) scummvm

dist:
	rm -f $(ZIPFILE)
	zip -q $(ZIPFILE) $(DISTFILES)

check:
$(OBJS): $(INCS)
