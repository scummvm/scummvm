# $Header$

ronindir = /usr/local/ronin

VPATH = ..

CC      = sh-elf-g++ -ml -m4-single-only
CFLAGS  = -O1 -Wno-multichar
DEFINES = -D__DC__ -DNONSTANDARD_PORT -DNONSTANDARD_SAVE
LDFLAGS := -Wl,-Ttext,0x8c010000 -nostartfiles ronin/crt0.o
INCLUDES:= -I./ -I../ -I../sound
CPPFLAGS= $(DEFINES) $(INCLUDES)
LIBS	= ronin/libronin.a ronin/libz.a -lm

INCS	= scumm.h scummsys.h stdafx.h portdefs.h dc.h

OBJS	= actor.o boxes.o costume.o gfx.o object.o resource.o \
	saveload.o script.o scummvm.o sound.o string.o \
	sys.o verbs.o script_v1.o script_v2.o debug.o gui.o \
	sound/imuse.o sound/fmopl.o sound/mixer.o debugrl.o \
	akos.o vars.o insane.o  gameDetector.o init.o mp3_cd.o \
	v3/resource_v3.o v4/resource_v4.o main.o \
	simon/midi.o simon/simon.o simon/simonsys.o sound/mididrv.o \
	dcmain.o display.o audio.o input.o selector.o icon.o \
	label.o vmsave.o

.cpp.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $(<) -o $*.o

all: scummvm

scummvm: $(OBJS) ronin
	$(CC) $(LDFLAGS) -o $(@) $(OBJS) $(LIBS)

$(OBJS): Makefile sound/.create simon/.create v3/.create v4/.create ronin

sound/.create:
	mkdir sound && touch $@

simon/.create:
	mkdir simon && touch $@

v3/.create:
	mkdir v3 && touch $@

v4/.create:
	mkdir v4 && touch $@

ronin:
	ln -s $(ronindir) $@

clean:
	rm -f $(OBJS) scummvm

check:
$(OBJS): $(INCS)
